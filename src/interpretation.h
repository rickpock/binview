#ifndef BINVIEW_INTERPRETATION
#define BINVIEW_INTERPRETATION

#include <string>
#include "byteIterator.h"

using namespace std;

enum Locale
{
    LOCALE_EN_US
};

class AscizInterpretation;
class AsciiInterpretation;
class HexInterpretation;

class Interpretation
{
public:
    virtual string format(IByteIterator&, Locale) = 0;

    static AscizInterpretation asciz;
    static AsciiInterpretation ascii;
    static HexInterpretation hex;
};

class AscizInterpretation : public Interpretation
{
public:
    string format(IByteIterator&, Locale);
};

class AsciiInterpretation : public Interpretation
{
public:
    string format(IByteIterator&, Locale);
};

class HexInterpretation : public Interpretation
{
public:
    string format(IByteIterator&, Locale);
};

#endif
