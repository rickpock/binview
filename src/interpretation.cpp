#include "interpretation.h"

Interpretation::Interpretation(string (*displayFunc)(const unsigned char*, Locale)) : display(displayFunc)
{
}

Interpretation::Interpretation()
{
}

void Interpretation::init()
{
    asciiz = Interpretation(Interpretation::displayAsciiz);
}

string Interpretation::displayAsciiz(const unsigned char* data, Locale locale)
{
    return (char *)data;
}

Interpretation Interpretation::asciiz;
