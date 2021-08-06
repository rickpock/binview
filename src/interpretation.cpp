#include "interpretation.h"

Interpretation::Interpretation(void (*displayFunc)(const unsigned char*, Locale, char*)) : display(displayFunc)
{
}

Interpretation::Interpretation()
{
}

void Interpretation::init()
{
    asciiz = Interpretation(Interpretation::displayAsciiz);
}

void Interpretation::displayAsciiz(const unsigned char* data, Locale locale, char * out)
{
    strcpy(out, (const char*)data);
}

Interpretation Interpretation::asciiz;
