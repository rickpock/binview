#include "interpretation.h"

Interpretation::Interpretation(string (*formatFunc)(const unsigned char*, Locale)) : format(formatFunc)
{
}

Interpretation::Interpretation()
{
}

void Interpretation::init()
{
    asciz = Interpretation(Interpretation::formatAsciz);
}

string Interpretation::formatAsciz(const unsigned char* data, Locale locale)
{
    return (char *)data;
}

Interpretation Interpretation::asciz;
