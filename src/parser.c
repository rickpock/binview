#include "parser.h"

#include <stdlib.h>
#include <string.h>

#include "color.h"

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
long readCentralDirectoryFileHeader(FILE *fp, long offset, Node *parentNode);
long readEndOfCentralDirectoryRecord(FILE *fp, long offset, Node *parentNode);

Node *parse(FILE *fp)
{
    // Length will be updated at the end of the function
    Node *output = newNode(NONE, "Zip File", (Segment[]){{.offset = 0, .length = 0}}, 1);

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
            offset += readCentralDirectoryFileHeader(fp, offset, output);
            continue;
        }

        if (memcmp(signatureBuffer, "\x50\x4b\x05\x06", 4) == 0)
        {
            offset += readEndOfCentralDirectoryRecord(fp, offset, output);
            continue;
        }

        // Section unrecognized
        break;
    }

    output->segments[0].length = offset;

    return output;
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

    Node *headerNode = newNode(GREEN, "Local File Header", (Segment[]){{.offset = offset, .length = localFileHeaderLen}}, 1);
    addChildNode(parentNode, headerNode);
    Node *dataNode = newNode(LIGHT_BLUE, "File Data", (Segment[]){{.offset = offset + localFileHeaderLen, .length = compressedSize}}, 1);
    addChildNode(parentNode, dataNode);

    addChildNode(headerNode,
        newContigNode(GREEN, "Signature", offset + 0x0, 0x4));
    addChildNode(headerNode,
        newContigNode(LIGHT_GREEN, "Version", offset + 0x4, 0x2));
    addChildNode(headerNode,
        newContigNode(LIGHT_CYAN, "Flags", offset + 0x6, 0x2));
    addChildNode(headerNode,
        newContigNode(LIGHT_PURPLE, "Compression method", offset + 0x8, 0x2));
    addChildNode(headerNode,
        newContigNode(CYAN, "File modification time", offset + 0xA, 0x2));
    addChildNode(headerNode,
        newContigNode(LIGHT_GREEN, "File modification date", offset + 0xC, 0x2));
    addChildNode(headerNode,
        newContigNode(LIGHT_CYAN, "CRC-32 checksum", offset + 0xE, 0x4));
    addChildNode(headerNode,
        newContigNode(LIGHT_PURPLE, "Compressed size", offset + 0x12, 0x4));
    addChildNode(headerNode,
        newContigNode(CYAN, "Uncompressed size", offset + 0x16, 0x4));
    addChildNode(headerNode,
        newContigNode(LIGHT_GREEN, "File name length", offset + 0x1A, 0x2));
    addChildNode(headerNode,
        newContigNode(LIGHT_CYAN, "Extra field length", offset + 0x1C, 0x2));
    addChildNode(headerNode,
        newContigNode(LIGHT_PURPLE, "File name", offset + 0x1E, fileNameLen));
    addChildNode(headerNode,
        newContigNode(CYAN, "Extra field", offset + 0x1E + fileNameLen, extraFieldLen));

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

    Node *headerNode = newNode(WHITE, "Central Directory File Header", (Segment[]){{.offset = offset, .length = centralDirectoryFileHeaderLen}}, 1);
    addChildNode(parentNode, headerNode);

    fseek(fp, centralDirectoryFileHeaderLen, SEEK_CUR);

    return centralDirectoryFileHeaderLen;
}

long readEndOfCentralDirectoryRecord(FILE *fp, long offset, Node *parentNode)
{
    short commentLen;

    peekRelative(fp, 0x14, 2, (char *)&commentLen);    // TODO: Error checking

    int endOfCentralDirectoryRecordLen = 0x16 + commentLen;

    Node *eocdrNode = newNode(YELLOW, "End of Central Directory Record", (Segment[]){{.offset = offset, .length = endOfCentralDirectoryRecordLen}}, 1);
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