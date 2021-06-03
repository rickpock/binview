#include <stdlib.h>
#include <string.h>
#include "hierarchy.h"

void newNode(int color, char *description, Segment *segments, int segmentCnt, Node *children, int childCnt, Node *node)
{
    //node = (Node *)malloc(sizeof(Node));
    
    node->color = color;
    node->description = malloc(strlen(description) + 1);
    strcpy(node->description, description);

    if (segmentCnt)
    {
        node->segments = malloc(sizeof(Segment) * segmentCnt);
        memcpy(node->segments, segments, sizeof(Segment) * segmentCnt);
        node->segmentCnt = segmentCnt;
    } else {
        node->segments = NULL;
    }
    
    if (childCnt)
    {
        node->children = malloc(sizeof(Node) * childCnt);
        memcpy(node->children, children, sizeof(Node) * childCnt);
        node->childCnt = childCnt;
    } else {
        node->children = NULL;
    }
}

void deleteNode(Node node)
{
    free(node.description);
    if (node.segments)
    { free(node.segments); }
    if (node.children)
    {
        for (int childIdx = 0; childIdx < node.childCnt; childIdx++)
        {
            deleteNode(node.children[childIdx]);
        }
        free(node.children);
    }
}