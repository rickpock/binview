#include <stdio.h>
//#include <stdlib.h>

int main()
{
    FILE *fp;

    fp = fopen("test/resources/example", "r");

    if (fp == NULL)
    {
        perror("Uh-oh");
        return 1;
    }

    int isEof = 0;

    char row[17];
    row[16] = 0;
    while (! isEof)
    {
        for (int counter = 0; counter < 16; counter++)
        {
            row[counter] = fgetc(fp);
            if (row[counter] == EOF)
            {
                row[counter] = 0;
                isEof = EOF;
                break;
            }
        }
        
        printf("%02X %02X %02X %02X %02X %02X %02X %02X  ", row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[7]);
        printf("%02X %02X %02X %02X %02X %02X %02X %02X  ", row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[7]);
        printf("%s\n", row);
        /*
        while ((ch = fgetc(fp)) != EOF)
        {
            printf("%02X ", ch);
        }

        if (ch == EOF)
        { break; }*/
    }
    fclose(fp);

    return 0;
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