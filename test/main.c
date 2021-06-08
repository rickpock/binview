#include <stdio.h>

#include <termios.h>


#include "../src/hierarchy.h"
#include "../src/color.h"
#include "../src/parser.h"

void draw(FILE *fp, const Node *root, const Node *selected);

int expandNode(const Node *root, const Node *selected);

unsigned char toPrintableChar(unsigned char ch);
int read16(FILE *fp, unsigned char* buffer);
void printHeader();
void print16(unsigned char* buffer, int bufferSz, long offset, int colors[]);

void setColor(enum printColor color);

void findColors(const Node *rootNode, const Node *selected, long offset, long length, int *result);
void findColorsRecur(const Node *rootNode, const Node *selected, long offset, long length, int *result);

void printHierarchy(const Node *rootNode, const Node *selected);
void printHierarchyRecur(const Node *node, const Node *selected, int depth);

int main(int argc, char **argv)
{
    struct termios info;
    tcgetattr(0, &info);          /* get current terminal attirbutes; 0 is the file descriptor for stdin */
    info.c_lflag &= ~ICANON;      /* disable canonical mode */
    info.c_cc[VMIN] = 1;          /* wait until at least one keystroke available */
    info.c_cc[VTIME] = 0;         /* no timeout */
    tcsetattr(0, TCSANOW, &info); /* set immediately */

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

    Node *root = parse(fp);

    Node *selected = root;

    while(1)
    {
        draw(fp, root, selected);

        char nextChar = getchar();
        switch (nextChar)
        {
            case 'q':
                fclose(fp);
                deleteNode(root);
                return 0;

            case '\033':
                getchar();
                nextChar = getchar();

                switch (nextChar)
                {
                    case 'A':   // Up
                        if (selected->prevSibling)
                        { selected = selected->prevSibling; }
                        break;

                    case 'B':   // Down
                        if (selected->nextSibling)
                        { selected = selected->nextSibling; }
                        break;
                        
                    case 'C':   // Right
                        if (selected->firstChild)
                        { selected = selected->firstChild; }
                        break;
                        
                    case 'D':   // Left
                        if (selected->parent)
                        { selected = selected->parent; }
                        break;
                        
                }
        }
    }
}

void draw(FILE *fp, const Node *root, const Node *selected)
{
    printf("\033[2J\n");

    fseek(fp, 0, SEEK_SET);

    const int BUFFER_SIZE = 16;
    unsigned char buffer[BUFFER_SIZE];
    int bytesRead = BUFFER_SIZE;
    long offset = 0;
    int colors[BUFFER_SIZE];

    printHeader();
    while ((bytesRead = read16(fp, buffer)) == BUFFER_SIZE)
    {
        findColors(root, selected, offset, BUFFER_SIZE, colors);

        print16(buffer, bytesRead, offset, colors);
        offset += BUFFER_SIZE;
    }
    findColors(root, selected, offset, BUFFER_SIZE, colors);
    print16(buffer, bytesRead, offset, colors);

    printf("\n");

    printHierarchy(root, selected);
    setColor(NONE);
}

// Expand root if it is an ancestor of selected
int expandNode(const Node *root, const Node *selected)
{
    // TODO: Redesign; super inefficient right now
    Node *child = root->firstChild;
    while(child)
    {
        if (child == selected)
        {
            return 1;
        }
        if (expandNode(child, selected))
        {
            return 1;
        }

        child = child->nextSibling;
    }

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

void printHeader()
{
    setColor(NONE);
    printf("          ");

    for (int colIdx = 0; colIdx < 8; colIdx++)
    {
        printf("%02X ", colIdx);
    }

    printf(" ");

    for (int colIdx = 8; colIdx < 16; colIdx++)
    {
        printf("%02X ", colIdx);
    }

    printf("\n");
}

inline void print16(unsigned char* buffer, int bufferSz, long offset, int colors[])
{
    setColor(NONE);
    printf("%08lX  ", offset);

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

void findColors(const Node *rootNode, const Node *selected, long offset, long length, int *result)
{
    for (long resultIdx = 0; resultIdx < length; resultIdx++)
    {
        result[resultIdx] = NONE;
    }

    findColorsRecur(rootNode, selected, offset, length, result);
}

void findColorsRecur(const Node *rootNode, const Node *selected, long offset, long length, int *result)
{
    for (long resultIdx = 0; resultIdx < length; resultIdx++)
    {
        if (rootNode == selected)
        {
            result[resultIdx] = LIGHT_BLUE;
        }
    }

    Node *childNode = rootNode->firstChild;
    while(childNode)
    {
        for (int segmentIdx = 0; segmentIdx < childNode->segmentCnt; segmentIdx++)
        {
            Segment segment = childNode->segments[segmentIdx];

            if (segment.offset < offset + length && segment.offset + segment.length > offset)
            {
                // TODO: Clean this up so it's actually understandable
                long startIdx = (segment.offset < offset) ? 0 : segment.offset - offset;
                long endIdx = (segment.offset + segment.length > offset + length) ? length : segment.offset + segment.length - offset;

                findColorsRecur(childNode, selected, startIdx + offset, endIdx - startIdx, &result[startIdx]);
            }
        }

        childNode = childNode->nextSibling;
    }
}

void printHierarchy(const Node *rootNode, const Node *selected)
{
    printHierarchyRecur(rootNode, selected, 0);
}

void printHierarchyRecur(const Node *node, const Node *selected, int depth)
{
    for (int depthIdx = 0; depthIdx < depth; depthIdx++)
    {
        printf("  ");
    }

    if (node == selected)
    {
        setColor(LIGHT_BLUE);
    } else {
        setColor(NONE);
    }
    printf("%s\n", node->description);

    if (expandNode(node, selected))
    {
        Node *childNode = node->firstChild;
        while(childNode)
        {
            printHierarchyRecur(childNode, selected, depth + 1);

            childNode = childNode->nextSibling;
        }
    }
}