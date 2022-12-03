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
    Node *output = new Node("Zip File", 0L, 0L, DT_NONE);

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

long readCentralDirectory(FILE *fp, long offset, Node *parentNode)
{
    Node *centralDirectory = new Node("Central Directory", offset, 0, DT_NONE);

    char signatureBuffer[4];

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

    centralDirectory->segments[0].length = offset - centralDirectory->segments[0].offset;

    return offset;
}

long readLocalFileHeader(FILE *fp, long offset, Node *parentNode)
{
    short fileNameLen;
    short extraFieldLen;
    long compressedSize;

    peekRelative(fp, 0x1a, 2, (char *)&fileNameLen);    // TODO: Error checking
    peekRelative(fp, 0x1c, 2, (char *)&extraFieldLen);    // TODO: Error checking
    peekRelative(fp, 0x12, 4, (char *)&compressedSize);    // TODO: Error checking

    int localFileHeaderLen = 0x1e + fileNameLen + extraFieldLen;

    Node *filenameNode;
    Node *headerNode = new Node("Local File Header", offset, localFileHeaderLen, DT_NODE);
    headerNode->displayInfo = (void *)filenameNode;
    addChildNode(parentNode, headerNode);
    Node *dataNode = new Node("File Data", offset + localFileHeaderLen, compressedSize, DT_NODE);
    dataNode->displayInfo = (void *)filenameNode;
    addChildNode(parentNode, dataNode);

    addChildNode(headerNode,
        new Node("Signature", offset + 0x0, 0x4, DT_HEX));
    addChildNode(headerNode,
        new Node("Version", offset + 0x4, 0x2, DT_INT | DT_INT_OPT_INCL_HEX));
    addChildNode(headerNode,
        new Node("Flags", offset + 0x6, 0x2, DT_FLAGS));
    addChildNode(headerNode,
        new Node("Compression method", offset + 0x8, 0x2, DT_INT | DT_INT_OPT_INCL_HEX));
    addChildNode(headerNode,
        new Node("File modification time", offset + 0xA, 0x2, DT_CUSTOM_MSDOS_TIME));
    addChildNode(headerNode,
        new Node("File modification date", offset + 0xC, 0x2, DT_CUSTOM_MSDOS_DATE));
    addChildNode(headerNode,
        new Node("CRC-32 checksum", offset + 0xE, 0x4, DT_HEX));
    addChildNode(headerNode,
        new Node("Compressed size", offset + 0x12, 0x4, DT_INT | DT_INT_OPT_INCL_HEX));
    addChildNode(headerNode,
        new Node("Uncompressed size", offset + 0x16, 0x4, DT_INT | DT_INT_OPT_INCL_HEX));
    addChildNode(headerNode,
        new Node("File name length", offset + 0x1A, 0x2, DT_INT | DT_INT_OPT_INCL_HEX));
    addChildNode(headerNode,
        new Node("Extra field length", offset + 0x1C, 0x2, DT_INT | DT_INT_OPT_INCL_HEX));
    filenameNode = new Node("File name", offset + 0x1E, fileNameLen, DT_ASCII);
    addChildNode(headerNode, filenameNode);
    addChildNode(headerNode,
        new Node("Extra field", offset + 0x1E + fileNameLen, extraFieldLen, DT_HEX));

    fseek(fp, localFileHeaderLen + compressedSize, SEEK_CUR);

    return localFileHeaderLen + compressedSize;
}

long readCentralDirectoryFileHeader(FILE *fp, long offset, Node *parentNode)
{
    short fileNameLen;
    short extraFieldLen;
    short fileCommentLen;

    peekRelative(fp, 0x1c, 2, (char *)&fileNameLen);    // TODO: Error checking
    peekRelative(fp, 0x1e, 2, (char *)&extraFieldLen);    // TODO: Error checking
    peekRelative(fp, 0x20, 2, (char *)&fileCommentLen);    // TODO: Error checking

    int centralDirectoryFileHeaderLen = 0x2e + fileNameLen + extraFieldLen + fileCommentLen;

    Node *headerNode = new Node("Central Directory File Header", offset, centralDirectoryFileHeaderLen, DT_NONE);
    addChildNode(parentNode, headerNode);

    // TODO: Set DisplayTypes
    addChildNode(headerNode,
        new Node("Signature", offset + 0x0, 0x4, DT_NONE));
    addChildNode(headerNode,
        new Node("Version", offset + 0x4, 0x2, DT_NONE));
    addChildNode(headerNode,
        new Node("Version needed", offset + 0x6, 0x2, DT_NONE));
    addChildNode(headerNode,
        new Node("Flags", offset + 0x8, 0x2, DT_NONE));
    addChildNode(headerNode,
        new Node("Compression method", offset + 0xA, 0x2, DT_NONE));
    addChildNode(headerNode,
        new Node("File modification time", offset + 0xC, 0x2, DT_NONE));
    addChildNode(headerNode,
        new Node("File modification date", offset + 0xE, 0x2, DT_NONE));
    addChildNode(headerNode,
        new Node("CRC-32 checksum", offset + 0x10, 0x4, DT_NONE));
    addChildNode(headerNode,
        new Node("Compressed size", offset + 0x14, 0x4, DT_NONE));
    addChildNode(headerNode,
        new Node("Uncompressed size", offset + 0x18, 0x4, DT_NONE));
    addChildNode(headerNode,
        new Node("File name length", offset + 0x1C, 0x2, DT_NONE));
    addChildNode(headerNode,
        new Node("Extra field length", offset + 0x1E, 0x2, DT_NONE));
    addChildNode(headerNode,
        new Node("File comment length", offset + 0x20, 0x2, DT_NONE));
    addChildNode(headerNode,
        new Node("Disk # start", offset + 0x22, 0x2, DT_NONE));
    addChildNode(headerNode,
        new Node("Internal attributes", offset + 0x24, 0x2, DT_NONE));
    addChildNode(headerNode,
        new Node("External attributes", offset + 0x26, 0x4, DT_NONE));
    addChildNode(headerNode,
        new Node("Offset of local header", offset + 0x2A, 0x4, DT_NONE));
    addChildNode(headerNode,
        new Node("File name", offset + 0x2E, fileNameLen, DT_NONE));
    addChildNode(headerNode,
        new Node("Extra field", offset + 0x2E + fileNameLen, extraFieldLen, DT_NONE));
    addChildNode(headerNode,
        new Node("File comment", offset + 0x2E + fileNameLen + extraFieldLen, fileCommentLen, DT_NONE));

    fseek(fp, centralDirectoryFileHeaderLen, SEEK_CUR);

    return centralDirectoryFileHeaderLen;
}

long readEndOfCentralDirectoryRecord(FILE *fp, long offset, Node *parentNode)
{
    short commentLen;

    peekRelative(fp, 0x14, 2, (char *)&commentLen);    // TODO: Error checking

    int endOfCentralDirectoryRecordLen = 0x16 + commentLen;
    Node *eocdrNode = new Node("End of Central Directory Record", offset, endOfCentralDirectoryRecordLen, DT_NONE);
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
