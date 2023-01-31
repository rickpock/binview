#include "parser.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

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
    uint16_t fileNameLen;
    uint16_t extraFieldLen;
    uint32_t compressedSize;

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
    FlagsInterpretation* pDefaultFlagsInterp = new FlagsInterpretation{
            FlagsInterpretation::Flag(1, {"unencrypted", "encrypted"}), // bit 0
            FlagsInterpretation::Flag(2, "undefined"), // bits 1-2
            FlagsInterpretation::Flag(1, {"fields set in local header", "fields set in data descriptor"}), // bit 3
            FlagsInterpretation::Flag(1, "reserved"), // bit 4
            FlagsInterpretation::Flag(1, {"not compressed patched data", "compressed patched data"}), // bit 5
            FlagsInterpretation::Flag(1, {"no strong encryption", "strong encryption"}), // bit 6
            FlagsInterpretation::Flag(4, "unused"), // bits 7-10
            FlagsInterpretation::Flag(1, {"", "UTF-8 field encoding"}), // bit 11
            FlagsInterpretation::Flag(1, "reserved"), // bit 12
            FlagsInterpretation::Flag(1, {"", "Local Header fields masked"}), // bit 13
            FlagsInterpretation::Flag(1, "reserved"), // bit 14
            FlagsInterpretation::Flag(1, "reserved") // bit 15
        };
    FlagsInterpretation* pMethod6FlagsInterp = new FlagsInterpretation{
            FlagsInterpretation::Flag(1, {"unencrypted", "encrypted"}), // bit 0
            FlagsInterpretation::Flag(1, {"4K sliding dict", "8k sliding dict"}), // bit 1
            FlagsInterpretation::Flag(1, {"2 Shannon-Fano trees", "3 Shannon-Fano trees"}), // bit 2
            FlagsInterpretation::Flag(1, {"fields set in local header", "fields set in data descriptor"}), // bit 3
            FlagsInterpretation::Flag(1, "reserved"), // bit 4
            FlagsInterpretation::Flag(1, {"not compressed patched data", "compressed patched data"}), // bit 5
            FlagsInterpretation::Flag(1, {"no strong encryption", "strong encryption"}), // bit 6
            FlagsInterpretation::Flag(4, "unused"), // bits 7-10
            FlagsInterpretation::Flag(1, {"", "UTF-8 field encoding"}), // bit 11
            FlagsInterpretation::Flag(1, "reserved"), // bit 12
            FlagsInterpretation::Flag(1, {"", "Local Header fields masked"}), // bit 13
            FlagsInterpretation::Flag(1, "reserved"), // bit 14
            FlagsInterpretation::Flag(1, "reserved") // bit 15
        };
    FlagsInterpretation* pMethod89FlagsInterp = new FlagsInterpretation{
            FlagsInterpretation::Flag(1, {"unencrypted", "encrypted"}), // bit 0
            FlagsInterpretation::Flag(2, {"normal compression", "maximum compression", "fast compression", "super fast compression"}), // bits 1-2
            FlagsInterpretation::Flag(1, {"fields set in local header", "fields set in data descriptor"}), // bit 3
            FlagsInterpretation::Flag(1, "reserved"), // bit 4
            FlagsInterpretation::Flag(1, {"not compressed patched data", "compressed patched data"}), // bit 5
            FlagsInterpretation::Flag(1, {"no strong encryption", "strong encryption"}), // bit 6
            FlagsInterpretation::Flag(4, "unused"), // bits 7-10
            FlagsInterpretation::Flag(1, {"", "UTF-8 field encoding"}), // bit 11
            FlagsInterpretation::Flag(1, "reserved"), // bit 12
            FlagsInterpretation::Flag(1, {"", "Local Header fields masked"}), // bit 13
            FlagsInterpretation::Flag(1, "reserved"), // bit 14
            FlagsInterpretation::Flag(1, "reserved") // bit 15
        };
    FlagsInterpretation* pMethod14FlagsInterp = new FlagsInterpretation{
            FlagsInterpretation::Flag(1, {"unencrypted", "encrypted"}), // bit 0
            FlagsInterpretation::Flag(1, {"no EOS marker", "EOS marker used"}), // bit 1
            FlagsInterpretation::Flag(1, "undefined"), // bit 2
            FlagsInterpretation::Flag(1, {"fields set in local header", "fields set in data descriptor"}), // bit 3
            FlagsInterpretation::Flag(1, "reserved"), // bit 4
            FlagsInterpretation::Flag(1, {"not compressed patched data", "compressed patched data"}), // bit 5
            FlagsInterpretation::Flag(1, {"no strong encryption", "strong encryption"}), // bit 6
            FlagsInterpretation::Flag(4, "unused"), // bits 7-10
            FlagsInterpretation::Flag(1, {"", "UTF-8 field encoding"}), // bit 11
            FlagsInterpretation::Flag(1, "reserved"), // bit 12
            FlagsInterpretation::Flag(1, {"", "Local Header fields masked"}), // bit 13
            FlagsInterpretation::Flag(1, "reserved"), // bit 14
            FlagsInterpretation::Flag(1, "reserved") // bit 15
        };
    Node *compressionNode = new Node("Compression method", 0x8, 0x2, new EnumInterpretation("unknown", IntInterpretation::OPT_EXCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN, {
            EnumInterpretation::Enum(0, "no compression"),
            EnumInterpretation::Enum(1, "shrunk"),
            EnumInterpretation::Enum(2, "reduced with compression factor 1"),
            EnumInterpretation::Enum(3, "reduced with compression factor 2"),
            EnumInterpretation::Enum(4, "reduced with compression factor 3"),
            EnumInterpretation::Enum(5, "reduced with compression factor 4"),
            EnumInterpretation::Enum(6, "imploded"),
            EnumInterpretation::Enum(7, "reserved"),
            EnumInterpretation::Enum(8, "deflated"),
            EnumInterpretation::Enum(9, "enhanced deflated"),
            EnumInterpretation::Enum(10, "PKWare DCL imploded"),
            EnumInterpretation::Enum(11, "reserved"),
            EnumInterpretation::Enum(12, "compressed using BZIP2"),
            EnumInterpretation::Enum(13, "reserved"),
            EnumInterpretation::Enum(14, "LZMA"),
            EnumInterpretation::Enum(15, "reserved"),
            EnumInterpretation::Enum(16, "reserved"),
            EnumInterpretation::Enum(17, "reserved"),
            EnumInterpretation::Enum(18, "compressed using IBM TERSE"),
            EnumInterpretation::Enum(19, "IBM LZ77 z"),
            EnumInterpretation::Enum(98, "PPMd version I, Rev 1")
        }));
    addChildNode(headerNode,
        new Node("Flags", 0x6, 0x2, new ConditionalInterpretation(compressionNode, pDefaultFlagsInterp, {
            ConditionalInterpretation::Condition(6, pMethod6FlagsInterp),
            ConditionalInterpretation::Condition(8, pMethod89FlagsInterp),
            ConditionalInterpretation::Condition(9, pMethod89FlagsInterp),
            ConditionalInterpretation::Condition(14, pMethod14FlagsInterp)
        })));
    addChildNode(headerNode,
        compressionNode);
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
    uint16_t fileNameLen;
    uint16_t extraFieldLen;
    uint16_t fileCommentLen;

    peekRelative(fp, 0x1c, 2, (char *)&fileNameLen);    // TODO: Error checking
    peekRelative(fp, 0x1e, 2, (char *)&extraFieldLen);    // TODO: Error checking
    peekRelative(fp, 0x20, 2, (char *)&fileCommentLen);    // TODO: Error checking

    int centralDirectoryFileHeaderLen = 0x2e + fileNameLen + extraFieldLen + fileCommentLen;

    Node *filenameNode = new Node("File name", 0x2E, fileNameLen, Interpretation::ascii);

    Node *headerNode = new Node("Central Directory File Header", parentOffset, centralDirectoryFileHeaderLen, new NodeInterpretation(filenameNode));
    addChildNode(parentNode, headerNode);

    addChildNode(headerNode,
        new Node("Signature", 0x0, 0x4, Interpretation::hex));
    Node *zipSpecVersionNode = new Node("ZIP specification version", 0x0, 0x1, Interpretation::hex); // TODO: The value/10 indicates the major version number, and the value mod 10 is the minor version number.  
    Node *versionMadeByNode = new Node("Version made by", 0x1, 0x1, new EnumInterpretation("Unknown", IntInterpretation::OPT_EXCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN, {
            EnumInterpretation::Enum(0, "MS-DOS and OS/2 (FAT / VFAT / FAT32 file systems)"),
            EnumInterpretation::Enum(1, "Amiga"),
            EnumInterpretation::Enum(2, "OpenVMS"),
            EnumInterpretation::Enum(3, "UNIX"),
            EnumInterpretation::Enum(4, "VM/CMS"),
            EnumInterpretation::Enum(5, "Atari ST"),
            EnumInterpretation::Enum(6, "OS/2 H.P.F.S."),
            EnumInterpretation::Enum(7, "Macintosh"),
            EnumInterpretation::Enum(8, "Z-System"),
            EnumInterpretation::Enum(9, "CP/M"),
            EnumInterpretation::Enum(10, "Windows NTFS"),
            EnumInterpretation::Enum(11, "MVS (OS/390 - Z/OS)"),
            EnumInterpretation::Enum(12, "VSE"),
            EnumInterpretation::Enum(13, "Acorn Risc"),
            EnumInterpretation::Enum(14, "VFAT"),
            EnumInterpretation::Enum(15, "alternative MVS"),
            EnumInterpretation::Enum(16, "BeOS"),
            EnumInterpretation::Enum(17, "Tandem"),
            EnumInterpretation::Enum(18, "OS/400"),
            EnumInterpretation::Enum(19, "OS X (Darwin)")
        }));
    Node *versionNode = new Node("Version", 0x4, 0x2, Interpretation::hex);
    addChildNode(versionNode, zipSpecVersionNode);
    addChildNode(versionNode, versionMadeByNode);
    addChildNode(headerNode, versionNode);
        //new Node("Version", 0x4, 0x2, Interpretation::hex)); // TODO: This can be broken down more
    addChildNode(headerNode,
        new Node("Version needed", 0x6, 0x2, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(headerNode,
        new Node("Flags", 0x8, 0x2, NULL)); // TODO
    addChildNode(headerNode,
        new Node("Compression method", 0xA, 0x2, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN))); // TODO: Change to enum
    addChildNode(headerNode,
        new Node("File modification time", 0xC, 0x2, Interpretation::msdosTime));
    addChildNode(headerNode,
        new Node("File modification date", 0xE, 0x2, Interpretation::msdosDate));
    addChildNode(headerNode,
        new Node("CRC-32 checksum", 0x10, 0x4, Interpretation::hex));
    addChildNode(headerNode,
        new Node("Compressed size", 0x14, 0x4, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(headerNode,
        new Node("Uncompressed size", 0x18, 0x4, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(headerNode,
        new Node("File name length", 0x1C, 0x2, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(headerNode,
        new Node("Extra field length", 0x1E, 0x2, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(headerNode,
        new Node("File comment length", 0x20, 0x2, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(headerNode,
        new Node("Disk # start", 0x22, 0x2, new IntInterpretation(IntInterpretation::OPT_EXCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(headerNode,
        new Node("Internal attributes", 0x24, 0x2, NULL)); // TODO: Flags
    addChildNode(headerNode,
        new Node("External attributes", 0x26, 0x4, NULL)); // TODO
    addChildNode(headerNode,
        new Node("Offset of local header", 0x2A, 0x4, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(headerNode, filenameNode);
    addChildNode(headerNode,
        new Node("Extra field", 0x2E + fileNameLen, extraFieldLen, Interpretation::hex)); // TODO: This can be broken down more
    addChildNode(headerNode,
        new Node("File comment", 0x2E + fileNameLen + extraFieldLen, fileCommentLen, Interpretation::ascii));

    fseek(fp, centralDirectoryFileHeaderLen, SEEK_CUR);

    return centralDirectoryFileHeaderLen;
}

long readEndOfCentralDirectoryRecord(FILE *fp, long parentOffset, Node *parentNode)
{
    uint16_t commentLen;

    peekRelative(fp, 0x14, 2, (char *)&commentLen);    // TODO: Error checking

    int endOfCentralDirectoryRecordLen = 0x16 + commentLen;

    Node *eocdrNode = new Node("End of Central Directory Record", parentOffset, endOfCentralDirectoryRecordLen, NULL);
    addChildNode(parentNode, eocdrNode);

    addChildNode(eocdrNode,
        new Node("Signature", 0x0, 0x4, Interpretation::hex));
    addChildNode(eocdrNode,
        new Node("Disk #", 0x4, 0x2, new IntInterpretation(IntInterpretation::OPT_EXCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(eocdrNode,
        new Node("Disk # w/ central directory", 0x6, 0x2, new IntInterpretation(IntInterpretation::OPT_EXCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(eocdrNode,
        new Node("Disk entries", 0x8, 0x2, new IntInterpretation(IntInterpretation::OPT_EXCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(eocdrNode,
        new Node("Total entries", 0xA, 0x2, new IntInterpretation(IntInterpretation::OPT_EXCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(eocdrNode,
        new Node("Central directory size", 0xC, 0x4, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(eocdrNode,
        new Node("Offset of central directory from starting disk", 0x10, 0x4, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(eocdrNode,
        new Node("Zip file comment length", 0x14, 0x2, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(eocdrNode,
        new Node("Zip file comment", 0x16, commentLen, Interpretation::ascii));
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
