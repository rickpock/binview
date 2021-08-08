#include "interpretation.h"

Interpretation::Interpretation(string (*formatFunc)(IByteIterator&, Locale)) : format(formatFunc)
{
}

Interpretation::Interpretation()
{
}

void Interpretation::init()
{
    asciz = Interpretation(Interpretation::formatAsciz);
}

string Interpretation::formatAsciz(IByteIterator& data, Locale locale)
{
    string out = "";
    while(data.hasNext())
    {
        out += data.next();
    }
    return out;
}

Interpretation Interpretation::asciz;
