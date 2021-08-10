#include "interpretation.h"

Interpretation::Interpretation(string (*formatFunc)(IByteIterator&, Locale)) : format(formatFunc)
{
}

Interpretation::Interpretation()
{
}

string Interpretation::formatAsciz(IByteIterator& data, Locale locale)
{
    string out = "";
    while(data.hasNext() && data.next())
    {
        out += data.next();
    }
    return out;
}

string Interpretation::formatAscii(IByteIterator& data, Locale locale)
{
    string out = "";
    while(data.hasNext())
    {
        out += data.next();
    }
    return out;
}

string Interpretation::formatHex(IByteIterator& data, Locale locale)
{
    string out = "0x";

    char buffer[3];
    while(data.hasNext())
    {
        sprintf(buffer, "%02X", data.next());
        out += buffer;
    }
    return out;
}

Interpretation Interpretation::asciz = Interpretation(Interpretation::formatAsciz);
Interpretation Interpretation::ascii = Interpretation(Interpretation::formatAscii);
Interpretation Interpretation::hex = Interpretation(Interpretation::formatHex);
