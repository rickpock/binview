#include <stdio.h>
//#include <stdlib.h>

unsigned char toPrintableChar(unsigned char ch);
int read16(FILE *fp, unsigned char* buffer);
void print16(unsigned char* buffer, int bufferSz);

enum printColor
{
    NONE = -1,
    BLACK = 0,
    RED = 1,
    GREEN = 2,
    BROWN = 3,
    BLUE = 4,
    PURPLE = 5,
    CYAN = 6,
    GRAY = 7,
    DARK_GRAY = 8 + 0,
    LIGHT_RED = 8 + 1,
    LIGHT_GREEN = 8 + 2,
    YELLOW = 8 + 3,
    LIGHT_BLUE = 8 + 4,
    LIGHT_PURPLE = 8 + 5,
    LIGHT_CYAN = 8 + 6,
    WHITE = 8 +7
};

void setColor(enum printColor color);


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

    const int BUFFER_SIZE = 16;
    unsigned char buffer[BUFFER_SIZE];
    int bytesRead = BUFFER_SIZE;
    while ((bytesRead = read16(fp, buffer)) == BUFFER_SIZE)
    {
        print16(buffer, bytesRead);
    }
    print16(buffer, bytesRead);
    fclose(fp);

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
        printf("\e[3%dm", color);
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

inline void print16(unsigned char* buffer, int bufferSz)
{
    for (int counter = 0; counter < 8; counter++)
    {
        if (counter >= bufferSz)
        {
            printf("   ");
        } else {
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
            printf("%02X ", buffer[counter]);
        }
    }

    printf("  ");

    for (int counter = 0; counter < 16 && counter < bufferSz; counter++)
    {
        printf("%c", toPrintableChar(buffer[counter]));
    }

    printf("\n");
}