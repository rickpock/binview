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
    // SCAFFOLDING CODE
    // TODO: Remove when pInterpretation is set directly
    if (displayType == DT_ASCIZ)
    {
        this->pInterpretation = Interpretation::asciz;
    } else if (displayType == DT_ASCII)
    {
        this->pInterpretation = Interpretation::ascii;
    } else if (displayType == DT_HEX)
    {
        this->pInterpretation = Interpretation::hex;
    } else if ((displayType & DT_CATEGORY) == DT_INT)
    {
        int opts = 0;
        if ((displayType & DT_INT_OPT_BIGENDIAN) == DT_INT_OPT_BIGENDIAN)
        { opts |= IntInterpretation::OPT_BIG_ENDIAN; }
        if ((displayType & DT_INT_OPT_INCL_HEX) != DT_INT_OPT_INCL_HEX)
        { opts |= IntInterpretation::OPT_EXCL_HEX; }

        this->pInterpretation = new IntInterpretation(opts);
    } else if ((displayType & DT_CUSTOM_MSDOS_DATE) == DT_CUSTOM_MSDOS_DATE)
    {
        this->pInterpretation = Interpretation::msdosDate;
    } else if ((displayType & DT_CUSTOM_MSDOS_TIME) == DT_CUSTOM_MSDOS_TIME)
    {
        this->pInterpretation = Interpretation::msdosTime;
    } else {
        this->pInterpretation = NULL;
    }
    // END OF SCAFFOLDING CODE
    
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
