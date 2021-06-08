#ifndef BINVIEW_HIERARCHY
#define BINVIEW_HIERARCHY

typedef struct
{
    long offset;
    long length;
} Segment;

typedef struct Node_t
{
    int color;
    char *description;

    Segment *segments;
    int segmentCnt;

    struct Node_t *firstChild;
    struct Node_t *lastChild;

    struct Node_t *parent;

    struct Node_t *nextSibling;    // Children are a linked list. The first child points to the next.
    struct Node_t *prevSibling;
} Node;

Node * newContigNode(int color, char *description, long offset, long length);
Node * newNode(int color, char *description, Segment *segments, int segmentCnt);

void addChildNode(Node *parent, Node *child);

void deleteNode(Node *node);

#endif