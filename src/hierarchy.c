#include <stdlib.h>
#include <string.h>
#include "hierarchy.h"

Node * newNode(char *description, long offset, long length)
{
    return newNodeEx(description, (Segment[]){{.offset = offset, .length = length}}, 1);
}

Node * newNodeEx(char *description, Segment *segments, int segmentCnt)
{
    Node *node = malloc(sizeof(Node));

    node->description = malloc(strlen(description) + 1);
    strcpy(node->description, description);

    if (segmentCnt)
    {
        node->segments = malloc(sizeof(Segment) * segmentCnt);
        memcpy(node->segments, segments, sizeof(Segment) * segmentCnt);
        node->segmentCnt = segmentCnt;
    } else {
        node->segmentCnt = 0;
        node->segments = NULL;
    }
    
    node->firstChild = NULL;
    node->lastChild = NULL;

    node->parent = NULL;

    node->nextSibling = NULL;
    node->prevSibling = NULL;

    return node;
}

void addChildNode(Node *parent, Node *child)
{
    child->parent = parent;

    if (parent->firstChild == NULL)
    {
        parent->firstChild = parent->lastChild = child;
        return;
    }

    parent->lastChild->nextSibling = child;
    child->prevSibling = parent->lastChild;
    parent->lastChild = child;
}

void deleteNode(Node *node)
{
    free(node->description);
    if (node->segments)
    { free(node->segments); }

    Node* nextChild = node->firstChild;
    Node* thisChild;
    while (nextChild)
    {
        thisChild = nextChild;
        nextChild = nextChild->nextSibling;
        deleteNode(thisChild);
    }

    free(node);
}