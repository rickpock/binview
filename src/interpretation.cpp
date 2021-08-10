#include "interpretation.h"

string AscizInterpretation::format(IByteIterator& data, Locale Locale)
{
    string out = "";
    while(data.hasNext() && data.next())
    {
        out += data.next();
    }
    return out;
}

string AsciiInterpretation::format(IByteIterator& data, Locale Locale)
{
    string out = "";
    while(data.hasNext())
    {
        out += data.next();
    }
    return out;
}

string HexInterpretation::format(IByteIterator& data, Locale Locale)
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

AscizInterpretation Interpretation::asciz = AscizInterpretation();
AsciiInterpretation Interpretation::ascii = AsciiInterpretation();
HexInterpretation Interpretation::hex = HexInterpretation();
