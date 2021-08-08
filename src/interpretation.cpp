#include "interpretation.h"

Interpretation::Interpretation(string (*formatFunc)(const byte*, Locale)) : format(formatFunc)
{
}

Interpretation::Interpretation()
{
}

void Interpretation::init()
{
    asciz = Interpretation(Interpretation::formatAsciz);
}

string Interpretation::formatAsciz(const byte* data, Locale locale)
{
    return (char *)data;
}

Interpretation Interpretation::asciz;
