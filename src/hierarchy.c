#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "hierarchy.h"

Node *newNode(int color, char *description, Segment *segments, int segmentCnt, Node *children, int childCnt)
{
    Node *new = (Node *)malloc(sizeof(Node));
    
    new->color = color;
    new->description = malloc(strlen(description) + 1);
    strcpy(new->description, description);

    if (segmentCnt)
    {
        new->segments = malloc(sizeof(Segment) * segmentCnt);
        memcpy(new->segments, segments, sizeof(Segment) * segmentCnt);
        new->segmentCnt = segmentCnt;
    } else {
        new->segments = NULL;
    }
    
    if (childCnt)
    {
        new->children = malloc(sizeof(Node) * childCnt);
        memcpy(new->children, children, sizeof(Node) * childCnt);
        new->childCnt = childCnt;
    } else {
        new->children = NULL;
    }

    return new;
}

void deleteNode(Node *node)
{
    free(node->description);
    if (node->segments)
    { free(node->segments); }
    if (node->children)
    { free(node->children); }

    free(node);
}