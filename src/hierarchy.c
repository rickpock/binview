#include <stdlib.h>
#include <string.h>
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

void findColors(Node rootNode, long offset, long length, int *result)
{
    for (int childIdx; childIdx < rootNode.childCnt; childIdx++)
    {
        Node childNode = rootNode.children[childIdx];
        
        for (int segmentIdx; segmentIdx < childNode.segmentCnt; segmentIdx++)
        {
            Segment segment = childNode.segments[segmentIdx];

            if (segment.offset < offset + length && segment.offset + segment.length > offset)
            {
                // TODO: Recur
                long startIdx = (segment.offset < offset) ? 0 : segment.offset - offset;
                long endIdx = (segment.offset + segment.length > offset + length) ? length : (offset + length) - (segment.offset + segment.length);

                for (long resultIdx = startIdx; resultIdx++; resultIdx++)
                {
                    result[resultIdx] = childNode.color;
                }
            }
        }
    }
}