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

// TODO
// WARNING: This interpretation only works up to "long" size
class IntInterpretation : public Interpretation
{
public:
    string format(IByteIterator&, Locale);

    enum OPTIONS {
        OPT_NONE = 0x0,

        OPT_MASK_ENDIAN = 0x01,
        OPT_LITTLE_ENDIAN = 0x00,
        OPT_BIG_ENDIAN = 0x01,

        OPT_MASK_HEX = 0x02,
        OPT_INCL_HEX = 0x00,
        OPT_EXCL_HEX = 0x02
    };

    IntInterpretation(int opts);

private:
    int opts;

    static bool isSystemLittleEndian();
};

#endif
