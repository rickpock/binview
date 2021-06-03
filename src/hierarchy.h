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
    struct Node_t *children;
    int childCnt;
} Node;

void newNode(int color, char *description, Segment *segments, int segmentCnt, Node *children, int childCnt, Node* node);

void deleteNode(Node node);

#endif