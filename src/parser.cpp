#include "parser.h"

#include <stdlib.h>
#include <string.h>

/*
 * Read up to n characters into *out, or until EOF
 * Returns actual number of characters read
 */
long read(FILE *fp, long n, char *out);
/*
 * Peek up to n characters into *out, or until EOF, keeping the cursor at the same position in fp when done
 * Returns actual number of characters read
 */
long peek(FILE *fp, long n, char *out);
/*
 * Peek up to n characters into *out at offset bytes from current cursor position
 * Returns actual number of characters read
 */
long peekRelative(FILE *fp, long offset, long n, char *out);

long readLocalFileHeader(FILE *fp, long offset, Node *parentNode);
long readCentralDirectory(FILE *fp, long offset, Node *parentNode);
long readCentralDirectoryFileHeader(FILE *fp, long offset, Node *parentNode);
long readEndOfCentralDirectoryRecord(FILE *fp, long offset, Node *parentNode);

Node *parse(FILE *fp)
{
    // Length will be updated at the end of the function
    Node *output = new Node("Zip File", 0L, 0L, NULL);

    char signatureBuffer[4];

    long offset = 0;

    while (! feof(fp))
    {
        peek(fp, 4, signatureBuffer);

        if (memcmp(signatureBuffer, "\x50\x4b\x03\x04", 4) == 0)
        {
            offset += readLocalFileHeader(fp, offset, output);
            continue;
        }

        if (memcmp(signatureBuffer, "\x50\x4b\x01\x02", 4) == 0)
        {
            offset += readCentralDirectory(fp, offset, output);
            continue;
        }

        if (memcmp(signatureBuffer, "\x50\x4b\x05\x06", 4) == 0)
        {
            offset += readCentralDirectory(fp, offset, output);
            continue;
        }

        // Section unrecognized
        break;
    }

    output->segments[0].length = offset;

    DataNode* dn = new DataNode(output, new FileAccessor(fp));

    return output;
}

long readCentralDirectory(FILE *fp, long parentOffset, Node *parentNode)
{
    Node *centralDirectory = new Node("Central Directory", parentOffset, 0, NULL);

    char signatureBuffer[4];

    long offset = 0;

    while (! feof(fp))
    {
        peek(fp, 4, signatureBuffer);

        if (memcmp(signatureBuffer, "\x50\x4b\x01\x02", 4) == 0)
        {
            offset += readCentralDirectoryFileHeader(fp, offset, centralDirectory);
            continue;
        }

        if (memcmp(signatureBuffer, "\x50\x4b\x05\x06", 4) == 0)
        {
            offset += readEndOfCentralDirectoryRecord(fp, offset, centralDirectory);
            continue;
        }

        // Section unrecognized
        break;
    }

    addChildNode(parentNode, centralDirectory);

    centralDirectory->segments[0].length = offset;

    return offset;
}

long readLocalFileHeader(FILE *fp, long parentOffset, Node *parentNode)
{
    short fileNameLen;
    short extraFieldLen;
    long compressedSize;

    peekRelative(fp, 0x1a, 2, (char *)&fileNameLen);    // TODO: Error checking
    peekRelative(fp, 0x1c, 2, (char *)&extraFieldLen);    // TODO: Error checking
    peekRelative(fp, 0x12, 4, (char *)&compressedSize);    // TODO: Error checking

    int localFileHeaderLen = 0x1e + fileNameLen + extraFieldLen;

    Node *filenameNode = new Node("File name", 0x1E, fileNameLen, Interpretation::ascii);
    Node *headerNode = new Node("Local File Header", parentOffset, localFileHeaderLen, new NodeInterpretation(filenameNode));
    addChildNode(parentNode, headerNode);
    Node *dataNode = new Node("File Data", parentOffset + localFileHeaderLen, compressedSize, new NodeInterpretation(filenameNode));
    addChildNode(parentNode, dataNode);

    addChildNode(headerNode,
        new Node("Signature", 0x0, 0x4, Interpretation::hex));
    addChildNode(headerNode,
        new Node("Version", 0x4, 0x2, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(headerNode,
        new Node("Flags", 0x6, 0x2, NULL));	// TODO: Flags
    addChildNode(headerNode,
        new Node("Compression method", 0x8, 0x2, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(headerNode,
        new Node("File modification time", 0xA, 0x2, Interpretation::msdosTime));
    addChildNode(headerNode,
        new Node("File modification date", 0xC, 0x2, Interpretation::msdosDate));
    addChildNode(headerNode,
        new Node("CRC-32 checksum", 0xE, 0x4, Interpretation::hex));
    addChildNode(headerNode,
        new Node("Compressed size", 0x12, 0x4, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(headerNode,
        new Node("Uncompressed size", 0x16, 0x4, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(headerNode,
        new Node("File name length", 0x1A, 0x2, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(headerNode,
        new Node("Extra field length", 0x1C, 0x2, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(headerNode, filenameNode);
    addChildNode(headerNode,
        new Node("Extra field", 0x1E + fileNameLen, extraFieldLen, Interpretation::hex));

    fseek(fp, localFileHeaderLen + compressedSize, SEEK_CUR);

    return localFileHeaderLen + compressedSize;
}

long readCentralDirectoryFileHeader(FILE *fp, long parentOffset, Node *parentNode)
{
    short fileNameLen;
    short extraFieldLen;
    short fileCommentLen;

    peekRelative(fp, 0x1c, 2, (char *)&fileNameLen);    // TODO: Error checking
    peekRelative(fp, 0x1e, 2, (char *)&extraFieldLen);    // TODO: Error checking
    peekRelative(fp, 0x20, 2, (char *)&fileCommentLen);    // TODO: Error checking

    int centralDirectoryFileHeaderLen = 0x2e + fileNameLen + extraFieldLen + fileCommentLen;

    Node *headerNode = new Node("Central Directory File Header", parentOffset, centralDirectoryFileHeaderLen, NULL);
    addChildNode(parentNode, headerNode);

    // TODO: Set DisplayTypes
    addChildNode(headerNode,
        new Node("Signature", 0x0, 0x4, NULL));
    addChildNode(headerNode,
        new Node("Version", 0x4, 0x2, NULL));
    addChildNode(headerNode,
        new Node("Version needed", 0x6, 0x2, NULL));
    addChildNode(headerNode,
        new Node("Flags", 0x8, 0x2, NULL));
    addChildNode(headerNode,
        new Node("Compression method", 0xA, 0x2, NULL));
    addChildNode(headerNode,
        new Node("File modification time", 0xC, 0x2, NULL));
    addChildNode(headerNode,
        new Node("File modification date", 0xE, 0x2, NULL));
    addChildNode(headerNode,
        new Node("CRC-32 checksum", 0x10, 0x4, NULL));
    addChildNode(headerNode,
        new Node("Compressed size", 0x14, 0x4, NULL));
    addChildNode(headerNode,
        new Node("Uncompressed size", 0x18, 0x4, NULL));
    addChildNode(headerNode,
        new Node("File name length", 0x1C, 0x2, NULL));
    addChildNode(headerNode,
        new Node("Extra field length", 0x1E, 0x2, NULL));
    addChildNode(headerNode,
        new Node("File comment length", 0x20, 0x2, NULL));
    addChildNode(headerNode,
        new Node("Disk # start", 0x22, 0x2, NULL));
    addChildNode(headerNode,
        new Node("Internal attributes", 0x24, 0x2, NULL));
    addChildNode(headerNode,
        new Node("External attributes", 0x26, 0x4, NULL));
    addChildNode(headerNode,
        new Node("Offset of local header", 0x2A, 0x4, NULL));
    addChildNode(headerNode,
        new Node("File name", 0x2E, fileNameLen, NULL));
    addChildNode(headerNode,
        new Node("Extra field", 0x2E + fileNameLen, extraFieldLen, NULL));
    addChildNode(headerNode,
        new Node("File comment", 0x2E + fileNameLen + extraFieldLen, fileCommentLen, NULL));

    fseek(fp, centralDirectoryFileHeaderLen, SEEK_CUR);

    return centralDirectoryFileHeaderLen;
}

long readEndOfCentralDirectoryRecord(FILE *fp, long parentOffset, Node *parentNode)
{
    short commentLen;

    peekRelative(fp, 0x14, 2, (char *)&commentLen);    // TODO: Error checking

    int endOfCentralDirectoryRecordLen = 0x16 + commentLen;
    Node *eocdrNode = new Node("End of Central Directory Record", parentOffset, endOfCentralDirectoryRecordLen, NULL);
    addChildNode(parentNode, eocdrNode);
    fseek(fp, endOfCentralDirectoryRecordLen, SEEK_CUR);

    return endOfCentralDirectoryRecordLen;
}

long read(FILE *fp, long n, char *out)
{
    for (int idx = 0; idx < n; idx++)
    {
        if (feof(fp))
        { return idx; }

        out[idx] = fgetc(fp);
    }

    return n;
}

long peek(FILE *fp, long n, char *out)
{
    long readCnt = read(fp, n, out);
    fseek(fp, -readCnt, SEEK_CUR);
    return readCnt;
}

long peekRelative(FILE *fp, long offset, long n, char *out)
{
    fseek(fp, offset, SEEK_CUR);    // TODO: Error checking
    long readCnt = peek(fp, n, out);
    fseek(fp, -offset, SEEK_CUR);   // TODO: Error checking
    return readCnt;
}
