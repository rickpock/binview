#include <stdio.h>
//#include <stdlib.h>

char toPrintableChar(char ch);
int read16(FILE *fp, char* buffer);
void print16(char* buffer, int bufferSz);

int main()
{
    FILE *fp;

    fp = fopen("test/resources/example", "r");

    if (fp == NULL)
    {
        perror("Uh-oh");
        return 1;
    }

    const int BUFFER_SIZE = 16;
    char buffer[BUFFER_SIZE];
    int bytesRead = BUFFER_SIZE;
    while ((bytesRead = read16(fp, buffer)) == BUFFER_SIZE)
    {
        print16(buffer, bytesRead);
    }
    print16(buffer, bytesRead);
    fclose(fp);

    return 0;
}

inline char toPrintableChar(char ch)
{
    if (ch < 0x20)
    { return '.'; }

    if (ch == 0x3F)
    { return '.'; }

    return ch;
}

/*
 * Reads up to 16 bytes from fp and stores them in buffer
 * Buffer must be at least 16 bytes in size
 * Returns the number of bytes written to buffer
 */
inline int read16(FILE *fp, char* buffer)
{
    for (int counter = 0; counter < 16; counter++)
    {
        buffer[counter] = fgetc(fp);
        if (buffer[counter] == EOF)
        {
            return counter;
        }
    }

    return 16;
}

inline void print16(char* buffer, int bufferSz)
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