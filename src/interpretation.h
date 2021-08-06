#ifndef BINVIEW_INTERPRETATION
#define BINVIEW_INTERPRETATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Locale
{
    LOCALE_EN_US
};

class Interpretation
{
public:
    void (*display)(const unsigned char*, Locale, char*);

    static void init();

private:
    Interpretation();
    Interpretation(void (*displayFunc)(const unsigned char*, Locale, char*));
    static void displayAsciiz(const unsigned char* data, Locale locale, char * out);

public:
    static Interpretation asciiz;
};

#endif
