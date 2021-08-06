#include "interpretation.h"

Interpretation::Interpretation(string (*formatFunc)(const unsigned char*, Locale)) : format(formatFunc)
{
}

Interpretation::Interpretation()
{
}

void Interpretation::init()
{
    asciiz = Interpretation(Interpretation::formatAsciiz);
}

string Interpretation::formatAsciiz(const unsigned char* data, Locale locale)
{
    return (char *)data;
}

Interpretation Interpretation::asciiz;
