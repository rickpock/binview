#include <stdlib.h>
#include <string.h>
#include "hierarchy.h"

#include <stdio.h>

// offset is relative to the start of the parent Node
Node::Node(const char *description, long offset, long length, Interpretation* pInterpretation) : dataNode(NULL)
{
    this->init(description, (Segment[]){{.offset = offset, .length = length}}, 1, pInterpretation);
}

void Node::init(const char *description, Segment *segments, int segmentCnt, Interpretation* pInterpretation)
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

    printf("%s: %X\n", description,pInterpretation);
    this->pInterpretation = pInterpretation;
    
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

IByteAccessor* DataNode::getAccessorForChildNode(Node* node)
{
    if (node->segmentCnt == 1)
    { return accessor->subset(node->segments[0].offset, node->segments[0].length); }

    IByteAccessor** subsetArr = (IByteAccessor**)malloc(sizeof(IByteAccessor*) * node->segmentCnt);
    for (int segIdx = 0; segIdx < node->segmentCnt; segIdx++)
    {
        subsetArr[segIdx] = accessor->subset(node->segments[segIdx].offset, node->segments[segIdx].length);
    }

    AggAccessor* out = new AggAccessor(subsetArr, node->segmentCnt);
    free(subsetArr);
    return out;
}

DataNode::DataNode(Node* node, IByteAccessor* accessor) : node(node), accessor(accessor)
{
    node->dataNode = this;

    Node* child = node->firstChild;
    DataNode* prevDataChild = NULL;
    while (child)
    {
        IByteAccessor* childAccessor = getAccessorForChildNode(child);
        DataNode* dataChild = new DataNode(child, childAccessor);

        if (prevDataChild == NULL)
        {
            firstChild = dataChild;
        } else {
            dataChild->prevSibling = prevDataChild;
            prevDataChild->nextSibling = dataChild;
        }
        lastChild = dataChild;
        dataChild->parent = this;

        prevDataChild = dataChild;
        child = child->nextSibling;
    }
}

DataNode* DataNode::findDescendant(Node* node)
{
    if (this->node == node)
    { return this; }

    for (DataNode* child = firstChild; child != NULL; child = child->nextSibling)
    {
        DataNode* found = child->findDescendant(node);
        if (found != NULL)
        { return found; }
    }
    return NULL;
}

