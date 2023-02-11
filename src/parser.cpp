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
long readExtraField(FILE *fp, long offset, Node *parentNode);
long readCentralDirectory(FILE *fp, long offset, Node *parentNode);
long readCentralDirectoryFileHeader(FILE *fp, long offset, Node *parentNode);
long readEndOfCentralDirectoryRecord(FILE *fp, long offset, Node *parentNode);

EnumInterpretation *compressionInterpretation = new EnumInterpretation("unknown", IntInterpretation::OPT_EXCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN, {
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
    EnumInterpretation::Enum(20, "deprecated"),
    EnumInterpretation::Enum(93, "Zstandard"),
    EnumInterpretation::Enum(94, "MP3"),
    EnumInterpretation::Enum(95, "XZ"),
    EnumInterpretation::Enum(96, "JPEG variant"),
    EnumInterpretation::Enum(97, "WavPack"),
    EnumInterpretation::Enum(98, "PPMd version I, Rev 1"),
    EnumInterpretation::Enum(99, "AE-x encryption marker")
});

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
    Node *compressionNode = new Node("Compression method", 0x8, 0x2, compressionInterpretation);
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

    long extraFieldOffset = 0;
    Node *extraFieldsNode = new Node("Extra fields", 0x1E + fileNameLen, extraFieldLen, Interpretation::hex);
    fseek(fp, 0x1E + fileNameLen, SEEK_CUR);
    while (extraFieldOffset < extraFieldLen)
    {
        extraFieldOffset += readExtraField(fp, extraFieldOffset, extraFieldsNode);
        fprintf(stderr, "extraFieldOffset: %lu", extraFieldOffset);
    }
    addChildNode(headerNode, extraFieldsNode);

    fseek(fp, compressedSize, SEEK_CUR);

    return localFileHeaderLen + compressedSize;
}

long readExtraField(FILE *fp, long parentOffset, Node *parentNode)
{
    uint16_t headerLen = 0x4;
    uint16_t dataLen;

    peekRelative(fp, 0x2, 2, (char *)&dataLen);	// TODO: Error checking

    Node *extraFieldHeaderIdNode = new Node("Header ID", 0x0, 0x2, new EnumInterpretation("unknown", IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN, {
        EnumInterpretation::Enum(0x0001, "Zip64 extended information extra field"),
        EnumInterpretation::Enum(0x0007, "AV Info"),
        EnumInterpretation::Enum(0x0008, "Reserved for extended language encoding data (PFS)"),
        EnumInterpretation::Enum(0x0009, "OS/2"),
        EnumInterpretation::Enum(0x000A, "NTFS"),
        EnumInterpretation::Enum(0x000C, "OpenVMS"),
        EnumInterpretation::Enum(0x000D, "UNIX"),
        EnumInterpretation::Enum(0x000E, "Reserved for file stream and fork descriptors"),
        EnumInterpretation::Enum(0x000F, "Patch Descriptor"),
        EnumInterpretation::Enum(0x0014, "PKCS#7 Store for X.509 Certificates"),
        EnumInterpretation::Enum(0x0015, "X.509 Certificate ID and Signature for individual file"),
        EnumInterpretation::Enum(0x0016, "X.509 Certificate ID for Central Directory"),
        EnumInterpretation::Enum(0x0017, "Strong Encryption Header"),
        EnumInterpretation::Enum(0x0018, "Record Management Controls"),
        EnumInterpretation::Enum(0x0019, "PKCS#7 Encryption Recipient Certificate List"),
        EnumInterpretation::Enum(0x0020, "Reserved for Timestamp Record"),
        EnumInterpretation::Enum(0x0021, "Policy Decryption Key Record"),
        EnumInterpretation::Enum(0x0022, "Smartcrypt Key Provider Record"),
        EnumInterpretation::Enum(0x0023, "Smartcrypt Policy Key Data Record"),
        EnumInterpretation::Enum(0x0065, "IBM S/390 (Z390), AS/400 (I400) attributes - uncompressed"),
        EnumInterpretation::Enum(0x0066, "Reserved for IBM S/390 (Z390), AS/400 (I400) attributes - compressed"),
        EnumInterpretation::Enum(0x4690, "POSZIP 4690 (reserved)"),
        EnumInterpretation::Enum(0x07C8, "Macintosh"),
        EnumInterpretation::Enum(0x1986, "Pixar USD header ID"),
        EnumInterpretation::Enum(0x2605, "ZipIt Macintosh"),
        EnumInterpretation::Enum(0x2705, "ZipIt Macintosh 1.3.5+"),
        EnumInterpretation::Enum(0x2805, "ZipIt Macintosh 1.3.5+"),
        EnumInterpretation::Enum(0x334D, "Info-ZIP Macintosh"),
        EnumInterpretation::Enum(0x4154, "Tandem"),
        EnumInterpretation::Enum(0x4341, "Acorn/SparkFS"),
        EnumInterpretation::Enum(0x4453, "Windows NT security descriptor (binary ACL)"),
        EnumInterpretation::Enum(0x4704, "VM/CMS"),
        EnumInterpretation::Enum(0x470F, "VMS"),
        EnumInterpretation::Enum(0x4854, "THEOS"),
        EnumInterpretation::Enum(0x4B46, "FWKCS MD5"),
        EnumInterpretation::Enum(0x4C41, "OS/2 access control list (text ACL)"),
        EnumInterpretation::Enum(0x4D49, "Info-ZIP OpenVMS"),
        EnumInterpretation::Enum(0x4D63, "Macintosh Smartzip"),
        EnumInterpretation::Enum(0x4F4C, "Xceed original location extra field"),
        EnumInterpretation::Enum(0x5356, "AOS/VS (ACL)"),
        EnumInterpretation::Enum(0x5455, "extended timestamp"),
        EnumInterpretation::Enum(0x554E, "Xceed unicode extra field"),
        EnumInterpretation::Enum(0x5855, "Info-ZIP UNIX (original, also OS/2, NT, etc)"),
        EnumInterpretation::Enum(0x6375, "Info-ZIP Unicode Comment Extra Field"),
        EnumInterpretation::Enum(0x6542, "BeOS/BeBox"),
        EnumInterpretation::Enum(0x6854, "THEOS"),
        EnumInterpretation::Enum(0x7075, "Info-ZIP Unicode Path Extra Field"),
        EnumInterpretation::Enum(0x7441, "AtheOS/Syllable"),
        EnumInterpretation::Enum(0x756E, "ASi UNIX"),
        EnumInterpretation::Enum(0x7855, "Info-ZIP UNIX (new)"),
        EnumInterpretation::Enum(0x7875, "Info-ZIP UNIX (newer UID/GID)"),
        EnumInterpretation::Enum(0xA11E, "Data Stream Alignment (Apache Commons-Compress)"),
        EnumInterpretation::Enum(0xA220, "Microsoft Open Packaging Growth Hint"),
        EnumInterpretation::Enum(0xCAFE, "Java JAR file Extra Field Header ID"),
        EnumInterpretation::Enum(0xD935, "Andriod ZIP Alignment Extra Field"),
        EnumInterpretation::Enum(0xE57A, "Korean ZIP code page info"),
        EnumInterpretation::Enum(0xFD4A, "SMS/QDOS"),
        EnumInterpretation::Enum(0x9901, "AE-x encryption structure"),
        EnumInterpretation::Enum(0x9902, "unknown")
    }));

    Node *extraFieldNode = new Node("Extra Field", parentOffset, headerLen + dataLen, new NodeInterpretation(extraFieldHeaderIdNode));
    addChildNode(parentNode, extraFieldNode);

    Node *extraFieldHeaderNode = new Node("Extra field header", 0x0, headerLen, Interpretation::hex);
    addChildNode(extraFieldNode, extraFieldHeaderNode);
    addChildNode(extraFieldHeaderNode, extraFieldHeaderIdNode);
    addChildNode(extraFieldHeaderNode,
        new Node("Data size", 0x2, 0x2, new IntInterpretation(IntInterpretation::OPT_INCL_HEX | IntInterpretation::OPT_LITTLE_ENDIAN)));
    addChildNode(extraFieldNode, // TODO: Break down data
        new Node("Extra field data", headerLen, dataLen, Interpretation::hex));

    fseek(fp, headerLen + dataLen, SEEK_CUR);

    return headerLen + dataLen;
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
    Node *compressionNode = new Node("Compression method", 0xA, 0x2, compressionInterpretation);
    addChildNode(headerNode,
        new Node("Flags", 0x8, 0x2, new ConditionalInterpretation(compressionNode, pDefaultFlagsInterp, {
            ConditionalInterpretation::Condition(6, pMethod6FlagsInterp),
            ConditionalInterpretation::Condition(8, pMethod89FlagsInterp),
            ConditionalInterpretation::Condition(9, pMethod89FlagsInterp),
            ConditionalInterpretation::Condition(14, pMethod14FlagsInterp)
        })));
    addChildNode(headerNode, compressionNode);
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
        new Node("Internal attributes", 0x24, 0x2, Interpretation::hex));
    addChildNode(headerNode,
        new Node("External attributes", 0x26, 0x4, Interpretation::hex));
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
