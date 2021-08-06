#ifndef BINVIEW_HIERARCHY
#define BINVIEW_HIERARCHY

typedef struct
{
    long offset;
    long length;
} Segment;

typedef enum DisplayType_t
{
    DT_NONE = 0x00,

    // ASCII character strings
    DT_ASCII = 0x10,            // Print each byte to the screen as an ASCII character
    DT_ASCII_OPT_NULL_TERM = 0x01,  // Stop printing the memory at the first \0 character
    DT_ASCII_OPT_UNPRINTABLE_AS_HEX = 0x00,     // Display unprintable characters as their two-digit hex value, preceded by '\'. Display backslash (\) as '\\'.
    DT_ASCII_OPT_UNPRINTABLE_AS_PERIOD = 0x02,  // Display unprintable characters as '.'.

    DT_ASCIZ = DT_ASCII | DT_ASCII_OPT_NULL_TERM,   // Print the memory to the screen as a null-terminated ASCII string

    // Integers
    // INT only supports a single segment on the node, and only up to eight bytes
    // If multiple segments are used or more than eight bytes, the value displayed is truncated to the eight least-significant bytes of the first segment
    DT_INT = 0x20,              // Print the memory as an integer. Little endian is used unless DT_INT_OPT_BIGENDIAN flag is set
    DT_INT_OPT_LITTLEENDIAN = 0x00,
    DT_INT_OPT_BIGENDIAN = 0x01,
    DT_INT_OPT_EXCL_HEX = 0x00,
    DT_INT_OPT_INCL_HEX = 0x02, // Next to the decimal value, print 0x followed by the hex value of each byte, from most-significant byte to least-significant byte

    DT_INT_HEX = DT_INT | DT_INT_OPT_INCL_HEX,

    // Hex values
    DT_HEX = 0x30,              // Print 0x followed by the hex value of each byte, in order of memory (default)
    DT_HEX_OPT_MEM_ORDER = 0x00,
    DT_HEX_OPT_REVERSE_MEM_ORDER = 0x01,    // Print higher memory bytes first. This makes sense if the memory is little endian and you want to display it most-significant byte first, as is typical for numbers.
                                            // If considering this option, you may want to use DT_INT | DT_INT_OPT_INCL_HEX

    // Flags where each bit or combination of bits has a meaning
    DT_FLAGS = 0x40,            // Print each bit and what it means

    // Enums where each possible value has a meaning
    DT_ENUMS = 0x50,

    DT_NODE = 0x60, // Use the value of a child node

    // A custom bitwise interpretation -- Not yet implemented
    DT_BIT_CUSTOM = 0x70,       // TODO

    // Specific common data formats
    DT_CUSTOM_UNIX_TIMESTAMP = 0x80,
    DT_CUSTOM_MSDOS_DATE = 0x81,
    DT_CUSTOM_MSDOS_TIME = 0x82,

    DT_CATEGORY = 0xF0,
    DT_OPTIONS = 0x0F
} DisplayType;

typedef struct DisplayInfoFlags_t
{
    char *description;
    unsigned short lowBit;
    unsigned short highBit;

    struct DisplayInfoFlags_t *next;
} DisplayInfoFlags;

typedef struct Node_t
{
    char *description;

    Segment *segments;
    int segmentCnt;

    DisplayType displayType;
    // A variant holding additional info, depending on displayType
    // * For DT_NODE, displayInfo is a Node*
    // * For DT_FLAGS, displayInfo is a DisplayInfoFlags*
    void *displayInfo;  // If displayType is DT_NODE, displayInfo is a Node*

    struct Node_t *firstChild;
    struct Node_t *lastChild;

    struct Node_t *parent;

    struct Node_t *nextSibling;    // Children are a linked list. The first child points to the next.
    struct Node_t *prevSibling;
} Node;

Node * newNode(const char *description, long offset, long length, int displayType);
Node * newNodeEx(const char *description, Segment *segments, int segmentCnt, DisplayType displayType);

void addChildNode(Node *parent, Node *child);

void deleteNode(Node *node);

#endif
