#include <stdio.h>

#include "../src/hierarchy.h"
#include "../src/color.h"

unsigned char toPrintableChar(unsigned char ch);
int read16(FILE *fp, unsigned char* buffer);
void print16(unsigned char* buffer, int bufferSz, long offset, int colors[]);

void setColor(enum printColor color);

void findColors(Node rootNode, long offset, long length, int *result);

void printHierarchy(const Node rootNode);
void printHierarchyRecur(const Node node, int depth);

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        perror("Missing an argument.");
        return 2;
    }

    FILE *fp;

    fp = fopen(argv[1], "r");

    if (fp == NULL)
    {
        perror("Unable to read file.");
        return 1;
    }

    const int NODE_CHILD_CNT = 4;
    Node children[NODE_CHILD_CNT];

    newNode(GREEN, "Local file header 1", (Segment[]){{ .offset = 0, .length = 69}}, 1, NULL, 0, &children[0]);
    newNode(BLUE, "File data 1", (Segment[]){{ .offset = 69, .length = 18}}, 1, NULL, 0, &children[1]);
    newNode(WHITE, "Central file header", (Segment[]){{ .offset = 87, .length = 81}}, 1, NULL, 0, &children[2]);
    newNode(YELLOW, "End of central directory record", (Segment[]){{ .offset = 168, .length = 22}}, 1, NULL, 0, &children[3]);

    Segment rootSegments[] = {{ .offset = 0, .length = 190}};
    Node root;
    newNode(NONE, "Zip file", rootSegments, 1, children, NODE_CHILD_CNT, &root);

    const int BUFFER_SIZE = 16;
    unsigned char buffer[BUFFER_SIZE];
    int bytesRead = BUFFER_SIZE;
    long offset = 0;
    int colors[BUFFER_SIZE];
    
    while ((bytesRead = read16(fp, buffer)) == BUFFER_SIZE)
    {
        findColors(root, offset, BUFFER_SIZE, colors);

        print16(buffer, bytesRead, offset, colors);
        offset += BUFFER_SIZE;
    }
    findColors(root, offset, BUFFER_SIZE, colors);
    print16(buffer, bytesRead, offset, colors);
    fclose(fp);

    printf("\n");

    printHierarchy(root);
    setColor(NONE);

    deleteNode(root);

    return 0;
}

void setColor(enum printColor color)
{
    if (color == NONE)
    {
        printf("\e[0m");
        return;
    }

    if (color & 0x8)
    {
        printf("\e[1;3%dm", color & 0x7);
    } else {
        printf("\e[0;3%dm", color);
    }
}

inline unsigned char toPrintableChar(unsigned char ch)
{
    if (ch < 0x20)
    { return '.'; }

    if (ch >= 0x7F)
    { return '.'; }

    return ch;
}

/*
 * Reads up to 16 bytes from fp and stores them in buffer
 * Buffer must be at least 16 bytes in size
 * Returns the number of bytes written to buffer
 */
inline int read16(FILE *fp, unsigned char* buffer)
{
    for (int counter = 0; counter < 16; counter++)
    {
        buffer[counter] = fgetc(fp);
        if (buffer[counter] == (unsigned char)EOF)
        {
            return counter;
        }
    }

    return 16;
}

inline void print16(unsigned char* buffer, int bufferSz, long offset, int colors[])
{
    setColor(NONE);
    printf("%04lX  ", offset);

    for (int counter = 0; counter < 8; counter++)
    {
        if (counter >= bufferSz)
        {
            printf("   ");
        } else {
            setColor(colors[counter]);
            printf("%02X ", buffer[counter]);
        }
    }

    printf(" ");

    for (int counter = 8; counter < 16; counter++)
    {
        if (counter >= bufferSz)
        {
            printf("   ");
        } else {
            setColor(colors[counter]);
            printf("%02X ", buffer[counter]);
        }
    }

    printf("  ");

    for (int counter = 0; counter < 16 && counter < bufferSz; counter++)
    {
        setColor(colors[counter]);
        printf("%c", toPrintableChar(buffer[counter]));
    }

    printf("\n");
}

void findColors(Node rootNode, long offset, long length, int *result)
{
    for (long resultIdx = 0; resultIdx < length; resultIdx++)
    {
        result[resultIdx] = rootNode.color;
    }

    for (int childIdx = 0; childIdx < rootNode.childCnt; childIdx++)
    {
        Node childNode = rootNode.children[childIdx];
        
        for (int segmentIdx = 0; segmentIdx < childNode.segmentCnt; segmentIdx++)
        {
            Segment segment = childNode.segments[segmentIdx];

            if (segment.offset < offset + length && segment.offset + segment.length > offset)
            {
                // TODO: Recur?
                long startIdx = (segment.offset < offset) ? 0 : segment.offset - offset;
                long endIdx = (segment.offset + segment.length > offset + length) ? length : segment.offset + segment.length - offset;

                for (long resultIdx = startIdx; resultIdx < endIdx; resultIdx++)
                {
                    result[resultIdx] = childNode.color;
                }
            }
        }
    }
}

void printHierarchy(const Node rootNode)
{
    printHierarchyRecur(rootNode, 0);
}

void printHierarchyRecur(const Node node, int depth)
{
    for (int depthIdx = 0; depthIdx < depth; depthIdx++)
    {
        printf("  ");
    }

    setColor(node.color);
    printf("%s\n", node.description);

    for (int childIdx = 0; childIdx < node.childCnt; childIdx++)
    {
        printHierarchyRecur(node.children[childIdx], depth + 1);
    }
}