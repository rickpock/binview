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

long readLocalFileHeader(FILE *fp, long offset, Node *out);

Node *parse(FILE *fp)
{
    Node *output = malloc(sizeof(Node));

    // TODO: actual value for length
    newNode(NONE, "Zip File", (Segment[]){{.offset = 0, .length = 0xB0}}, 1, output);
    // output->description = malloc(strlen("Zip File") + 1);
    // strcpy(output->description, "Zip File");

    char signatureBuffer[4];

    long offset = 0;

    while (! feof(fp))
    {
        peek(fp, 4, signatureBuffer);

        if (memcmp(signatureBuffer, "\x50\x4b\x03\x04", 4) == 0)
        {
            Node *localFileHeaderNode = malloc(sizeof(Node));
            offset += readLocalFileHeader(fp, offset, localFileHeaderNode);
            addChildNode(output, localFileHeaderNode);
        }

        // Section unrecognized
        break;
    }

    return output;
}

long readLocalFileHeader(FILE *fp, long offset, Node *out)
{
    short fileNameLen;
    short extraFieldLen;

    fseek(fp, 0x1a, SEEK_CUR);  // TODO: Error checking
    peek(fp, 2, (char *)&fileNameLen);        // TODO: Error checking
    fseek(fp, -0x1a, SEEK_CUR); // TODO: Error checking

    fseek(fp, 0x1c, SEEK_CUR);  // TODO: Error checking
    peek(fp, 2, (char *)&extraFieldLen);        // TODO: Error checking
    fseek(fp, -0x1c, SEEK_CUR); // TODO: Error checking

    int localFileHeaderLen = 0x3e + fileNameLen + extraFieldLen;

    newNode(GREEN, "Local File Header", (Segment[]){{.offset = offset, .length = localFileHeaderLen}}, 1, out);

    return localFileHeaderLen;
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