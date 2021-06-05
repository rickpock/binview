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
    struct Node_t *nextSibling;    // Children are a linked list. The first child points to the next.
    int childCnt;
} Node;

void newNode(int color, char *description, Segment *segments, int segmentCnt, Node* node);

void addChildNode(Node *parent, Node *child);

void deleteNode(Node node);

#endif