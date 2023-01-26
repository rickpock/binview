#ifndef BINVIEW_INTERPRETATION
#define BINVIEW_INTERPRETATION

#include <string>
#include <vector>
#include <initializer_list>
#include "byteIterator.h"
#include <inttypes.h>

class Interpretation;

#include "hierarchy.h"

using namespace std;

enum Locale
{
    LOCALE_EN_US
};

class Interpretation
{
public:
    virtual string format(IByteIterator&, Locale) = 0;

    static Interpretation* asciz;
    static Interpretation* ascii;
    static Interpretation* hex;
    static Interpretation* msdosDate;
    static Interpretation* msdosTime;
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
// WARNING: This interpretation only works up to 64 bits
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

    static uint64_t readAs64Bits(IByteIterator&, int opts);

private:
    int opts;

    static bool isSystemLittleEndian();
};

class MsdosDateInterpretation : public Interpretation
{
public:
    string format(IByteIterator&, Locale);
};

class MsdosTimeInterpretation : public Interpretation
{
public:
    string format(IByteIterator&, Locale);
};

class Flag;

class NodeInterpretation : public Interpretation
{
public:
    NodeInterpretation(Node* node);

    string format(IByteIterator&, Locale);

private:
    Node* node;
};

class FlagsInterpretation : public Interpretation
{
public:
    FlagsInterpretation(initializer_list<Flag> flags);

    string format(IByteIterator&, Locale);

private:
    vector<Flag> flags;
};

class Flag
{
public:
    // TODO: Use different term than "interpretation", which is already used to mean something else
    Flag(uint8_t numBits, initializer_list<string> flagValues);
    Flag(uint8_t numBits, string flagValue); // Sets all bit sequences to the same meaning -- commonly used for "unused" or "reserved" bits

    uint8_t getNumBits();
    string getInterpretation(unsigned int value);

private:
    uint8_t numBits;
    vector<string> flagValues;
};

class Condition;

// Allows for one of several interpretations to be selected based on the value of another node
class ConditionalInterpretation : public Interpretation
{
public:
    ConditionalInterpretation(Node* node, Interpretation* pDefault, initializer_list<Condition> conditions);

    string format(IByteIterator&, Locale);

private:
    Node* node;
    Interpretation* pDefault;
    vector<Condition> conditions;
};

class Condition
{
public:
    Condition(uint64_t valueMatch, Interpretation* pInterpretation);

    uint64_t getValueMatch();
    Interpretation* getpInterpretation();

private:
    uint64_t valueMatch;
    Interpretation* pInterpretation;
};

#endif
