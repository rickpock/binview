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
    DT_ASCII = 0x10,            // Print each byte to the screen as an ASCII character
    DT_ASCIZ = 0x11,            // Print the memory to the screen as a null-terminated ASCII string
    DT_INT_BIGENDIAN = 0x20,    // Print the memory as an integer, assuming it's stored in big endian
    DT_INT_LITTLEENDIAN = 0x21, // Print the memory as an integer, assuming it's stored in little endian
    DT_HEX = 0x30,              // Print 0x followed by the hex value of each byte
    DT_FLAGS = 0x40,            // Print each bit and what it means
    DT_BIT_CUSTOM = 0x50        // TODO
} DisplayType;

typedef struct Node_t
{
    char *description;

    Segment *segments;
    int segmentCnt;

    DisplayType displayType;

    struct Node_t *firstChild;
    struct Node_t *lastChild;

    struct Node_t *parent;

    struct Node_t *nextSibling;    // Children are a linked list. The first child points to the next.
    struct Node_t *prevSibling;
} Node;

Node * newNode(char *description, long offset, long length);
Node * newNodeEx(char *description, Segment *segments, int segmentCnt, DisplayType displayType);

void addChildNode(Node *parent, Node *child);

void deleteNode(Node *node);

#endif