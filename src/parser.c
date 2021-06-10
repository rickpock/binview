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
    Node *output = newNode("Zip File",  0, 0);

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

    return output;
}

long readCentralDirectory(FILE *fp, long offset, Node *parentNode)
{
    Node *centralDirectory = newNode("Central Directory", offset, 0);

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

    Node *headerNode = newNode("Local File Header", offset, localFileHeaderLen);
    addChildNode(parentNode, headerNode);
    Node *dataNode = newNode("File Data", offset + localFileHeaderLen, compressedSize);
    addChildNode(parentNode, dataNode);

    addChildNode(headerNode,
        newNodeEx("Signature", (Segment[]){{.offset = offset + 0x0, .length = 0x4}}, 1, DT_HEX));
    addChildNode(headerNode,
        newNode("Version", offset + 0x4, 0x2));
    addChildNode(headerNode,
        newNode("Flags", offset + 0x6, 0x2));
    addChildNode(headerNode,
        newNode("Compression method", offset + 0x8, 0x2));
    addChildNode(headerNode,
        newNode("File modification time", offset + 0xA, 0x2));
    addChildNode(headerNode,
        newNode("File modification date", offset + 0xC, 0x2));
    addChildNode(headerNode,
        newNode("CRC-32 checksum", offset + 0xE, 0x4));
    addChildNode(headerNode,
        newNode("Compressed size", offset + 0x12, 0x4));
    addChildNode(headerNode,
        newNode("Uncompressed size", offset + 0x16, 0x4));
    addChildNode(headerNode,
        newNode("File name length", offset + 0x1A, 0x2));
    addChildNode(headerNode,
        newNode("Extra field length", offset + 0x1C, 0x2));
    addChildNode(headerNode,
        newNodeEx("File name", (Segment[]){{.offset = offset + 0x1E, .length = fileNameLen}}, 1, DT_ASCII));
    addChildNode(headerNode,
        newNode("Extra field", offset + 0x1E + fileNameLen, extraFieldLen));

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

    Node *headerNode = newNode("Central Directory File Header", offset, centralDirectoryFileHeaderLen);
    addChildNode(parentNode, headerNode);

    addChildNode(headerNode,
        newNode("Signature", offset + 0x0, 0x4));
    addChildNode(headerNode,
        newNode("Version", offset + 0x4, 0x2));
    addChildNode(headerNode,
        newNode("Version needed", offset + 0x6, 0x2));
    addChildNode(headerNode,
        newNode("Flags", offset + 0x8, 0x2));
    addChildNode(headerNode,
        newNode("Compression method", offset + 0xA, 0x2));
    addChildNode(headerNode,
        newNode("File modification time", offset + 0xC, 0x2));
    addChildNode(headerNode,
        newNode("File modification date", offset + 0xE, 0x2));
    addChildNode(headerNode,
        newNode("CRC-32 checksum", offset + 0x10, 0x4));
    addChildNode(headerNode,
        newNode("Compressed size", offset + 0x14, 0x4));
    addChildNode(headerNode,
        newNode("Uncompressed size", offset + 0x18, 0x4));
    addChildNode(headerNode,
        newNode("File name length", offset + 0x1C, 0x2));
    addChildNode(headerNode,
        newNode("Extra field length", offset + 0x1E, 0x2));
    addChildNode(headerNode,
        newNode("File comment length", offset + 0x20, 0x2));
    addChildNode(headerNode,
        newNode("Disk # start", offset + 0x22, 0x2));
    addChildNode(headerNode,
        newNode("Internal attributes", offset + 0x24, 0x2));
    addChildNode(headerNode,
        newNode("External attributes", offset + 0x26, 0x4));
    addChildNode(headerNode,
        newNode("Offset of local header", offset + 0x2A, 0x4));
    addChildNode(headerNode,
        newNode("File name", offset + 0x2E, fileNameLen));
    addChildNode(headerNode,
        newNode("Extra field", offset + 0x2E + fileNameLen, extraFieldLen));
    addChildNode(headerNode,
        newNode("File comment", offset + 0x2E + fileNameLen + extraFieldLen, fileCommentLen));

    fseek(fp, centralDirectoryFileHeaderLen, SEEK_CUR);

    return centralDirectoryFileHeaderLen;
}

long readEndOfCentralDirectoryRecord(FILE *fp, long offset, Node *parentNode)
{
    short commentLen;

    peekRelative(fp, 0x14, 2, (char *)&commentLen);    // TODO: Error checking

    int endOfCentralDirectoryRecordLen = 0x16 + commentLen;

    Node *eocdrNode = newNode("End of Central Directory Record", offset, endOfCentralDirectoryRecordLen);
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