#ifndef BINVIEW_INTERPRETATION
#define BINVIEW_INTERPRETATION

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

using namespace std;

enum Locale
{
    LOCALE_EN_US
};

class Interpretation
{
public:
    string (*display)(const unsigned char*, Locale);

    static void init();

private:
    Interpretation();
    Interpretation(string (*displayFunc)(const unsigned char*, Locale));
    static string displayAsciiz(const unsigned char* data, Locale locale);

public:
    static Interpretation asciiz;
};

#endif
