#include <stdlib.h>
#include <string.h>
#include "hierarchy.h"

#include <stdio.h>

Node::Node(const char *description, long offset, long length, int displayType)
{
    this->init(description, (Segment[]){{.offset = offset, .length = length}}, 1, (DisplayType)displayType);
}

void Node::init(const char *description, Segment *segments, int segmentCnt, DisplayType displayType)
{
    this->description = (char*)malloc(strlen(description) + 1);
    strcpy(this->description, description);

    if (segmentCnt)
    {
        this->segments = (Segment *)malloc(sizeof(Segment) * segmentCnt);
        memcpy(this->segments, segments, sizeof(Segment) * segmentCnt);
        this->segmentCnt = segmentCnt;
    } else {
        this->segmentCnt = 0;
        this->segments = NULL;
    }

    this->displayType = displayType;
    this->displayInfo = NULL;
    
    this->firstChild = NULL;
    this->lastChild = NULL;

    this->parent = NULL;

    this->nextSibling = NULL;
    this->prevSibling = NULL;
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
