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

string MsdosDateInterpretation::format(IByteIterator& data, Locale Locale)
{
    // We assume two bytes of data for this interpretation
    union
    {
        byte buffer[2] = {0, 0};
        uint16_t value;
    };
    if (data.hasNext())
    { buffer[0] = data.next(); }
    if (data.hasNext())
    { buffer[1] = data.next(); }

    uint16_t year = 1980 + ((value >> 9) & ((1 << 7) - 1));
    uint16_t month = (value >> 5) & ((1 << 4) - 1);
    uint16_t day = value & ((1 << 5) - 1);

    return std::to_string(month) + "/" + std::to_string(day) + "/" + std::to_string(year);
}

string MsdosTimeInterpretation::format(IByteIterator& data, Locale Locale)
{
    // We assume two bytes of data for this interpretation
    union
    {
        byte buffer[2];
        uint16_t value;
    };
    if (data.hasNext())
    { buffer[0] = data.next(); }
    if (data.hasNext())
    { buffer[1] = data.next(); }

    uint16_t hour = (value >> 11) & ((1 << 5) - 1);
    uint16_t minute = (value >> 5) & ((1 << 6) - 1);
    uint16_t second = (value & ((1 << 5) - 1)) * 2;

    return std::to_string(hour) + ":" + std::to_string(minute) + ":" + std::to_string(second);
}

IntInterpretation::IntInterpretation(uint32_t opts) : opts(opts) {}

bool IntInterpretation::isSystemLittleEndian()
{
    short test = 1;
    return *(char *)&test;
}

// TODO: Switch to inttypes.h data type
uint64_t IntInterpretation::readAs64Bits(IByteIterator& data, uint32_t opts)
{
    uint64_t value = 0;

    // Read source from least-significant byte to most significant byte
    if ((opts & OPT_MASK_ENDIAN) == OPT_LITTLE_ENDIAN)
    {
        // Interpret data as Little Endian
        if (isSystemLittleEndian())
        {
            // Read source from left to right
            // Write into the data structure from left to right
            int valueIdx = 0;
            while (data.hasNext() && valueIdx < sizeof(uint64_t))
            {
                ((byte *)&value)[valueIdx] = data.next();
                valueIdx++;
            }
        } else {
            // Read source from left to right
            // Write into the data structure from right to left
            int valueIdx = sizeof(uint64_t);
            while (data.hasNext() && valueIdx >= 0)
            {
                ((byte *)&value)[valueIdx] = data.next();
                valueIdx--;
            }
        }
	
    } else {
        // Interpret data as Big Endian

        // Read source from right to left
        byte buffer[sizeof(uint64_t)];
        for (int bufferIdx = 0; bufferIdx < sizeof(uint64_t); bufferIdx++)
        {
            buffer[bufferIdx] = 0;
        }

        int bufferIdx = 0;
        while (data.hasNext())
        {
            buffer[bufferIdx] = data.next();
            bufferIdx = (bufferIdx + 1) % sizeof(uint64_t);
        }

        if (isSystemLittleEndian())
        {
            // Write into the data structure from left to right
            for (int valueIdx = 0; valueIdx < sizeof(uint64_t); valueIdx++)
            {
                // Read from buffer backwards (right to left), starting from (bufferIdx - 1)
                bufferIdx = (bufferIdx - 1 + sizeof(uint64_t)) % sizeof(uint64_t);

                ((byte *)&value)[valueIdx] = buffer[bufferIdx];
            }
        } else {
            // Write into the data structure from right to left
            for (int valueIdx = sizeof(uint64_t); valueIdx > 0; valueIdx--)
            {
                // Read from buffer backwards (right to left), starting from (bufferIdx - 1)
                bufferIdx = (bufferIdx - 1 + sizeof(uint64_t)) % sizeof(uint64_t);

                ((byte *)&value)[valueIdx] = buffer[bufferIdx];
            }
        }
    }

    return value;
}

// TODO
// WARNING: This interpretation only works up to 64 bits
string IntInterpretation::format(IByteIterator& data, Locale locale)
{
    string out = "";

    uint64_t value = readAs64Bits(data, opts);

    // Read source from least-significant byte to most significant byte
    // Write into the output memory from least significant byte to most significant byte
    if ((opts & OPT_MASK_ENDIAN) == OPT_LITTLE_ENDIAN)
    {
        // Interpret data as Little Endian

        out = std::to_string(value);
	
        // Print hex right to left when little endian
        if ((opts & OPT_MASK_HEX) == OPT_INCL_HEX)
        {
            byte buffer[sizeof(uint16_t)];
            for (int bufferIdx = 0; bufferIdx < sizeof(uint64_t); bufferIdx++)
            {
                buffer[bufferIdx] = 0;
            }

            int bufferIdx = 0;
            int dataLen = 0;

            data.reset();
            while (data.hasNext())
            {
                buffer[bufferIdx] = data.next();
                bufferIdx = (bufferIdx + 1) % sizeof(uint64_t);
                dataLen++;
            }

            out = out + " (0x";
    
            char charBuffer[3];
            for (int dataIdx = 0; dataIdx < sizeof(uint64_t) && dataIdx < dataLen; dataIdx++)
            {
                // Read from buffer backwards (right to left), starting from (bufferIdx - 1)
                bufferIdx = (bufferIdx - 1 + sizeof(uint64_t)) % sizeof(uint64_t);

                sprintf(charBuffer, "%02X", buffer[bufferIdx]);
                out += charBuffer;
            }
    
            out = out + ")";
        }
    } else {
        // Interpret data as Big Endian

        out = std::to_string(value);
	
        // Print hex left to right when big endian
        if ((opts & OPT_MASK_HEX) == OPT_INCL_HEX)
        {
            out = out + " (0x";
    
            char buffer[3];
            data.reset();
            while(data.hasNext())
            {
                sprintf(buffer, "%02X", data.next());
                out += buffer;
            }
    
            out = out + ")";
        }
    }

    return out;
}

NodeInterpretation::NodeInterpretation(Node* node) : node(node) {}

string NodeInterpretation::format(IByteIterator& data, Locale locale)
{
    Interpretation* nodeInterpretation = node->pInterpretation;
    IByteIterator* itr = node->dataNode->accessor->iterator();

    return nodeInterpretation->format(*itr, locale);
}

AdvancedNodeInterpretation::AdvancedNodeInterpretation(string fmtString, initializer_list<Node*> nodes) : fmtString(fmtString), nodes(nodes) {}

// Outputs "fmtString", but with string sequences '$N' where N is a number is replaced by the interpretation of the Nth node in the initializer list.
// Also, '$$' is replaced with '$'
// Only '$1' through '$9' are supported
string AdvancedNodeInterpretation::format(IByteIterator& data, Locale locale)
{
    vector<string> interpretations;

    for (vector<Node*>::iterator iter = nodes.begin(); iter < nodes.end(); iter++)
    {
        // TODO: Improve variable names. In particular avoid two usages of iter/itr and interpretation.
        Interpretation *pInterpretation = (*iter)->pInterpretation;
        IByteIterator *pItr = (*iter)->dataNode->accessor->iterator();
        interpretations.push_back(pInterpretation->format(*pItr, locale));
    }

    string out = "";
    char flag = '$';
    bool isFlag = false;
    const char *fmtStringData = fmtString.c_str();
    for (uint16_t fmtIdx = 0; fmtStringData[fmtIdx]; fmtIdx++)
    {
        char curChar = fmtStringData[fmtIdx];

        if (curChar == flag)
        {
            if (isFlag)
            {
                out += flag;
                isFlag = false;
            } else {
                isFlag = true;
            }
            continue;
        }

        if (isFlag)
        {
            unsigned char flagId = curChar - '1'; // TODO: only flags in range [1,9] are acceptable. Error otherwise.
            out += interpretations.at(flagId);
            isFlag = false;
            continue;
        }

        out += curChar;
    }

    return out;
}
    

FlagsInterpretation::FlagsInterpretation(initializer_list<Flag> flags) : flags(flags) {}

string FlagsInterpretation::format(IByteIterator& data, Locale locale)
{
    // WARNING: Assumes a flag has 64 bits maximum
    uint64_t buffer = 0;
    byte* pBuffer = (byte*)&buffer;
    uint8_t totalNumBits = 0;
    for (unsigned int bufferIdx = 0; data.hasNext(); bufferIdx++)
    {
        pBuffer[bufferIdx] = data.next();
        totalNumBits += 8;
    }

    string out = "";

    int8_t startBitIdx = totalNumBits - 1;
    for (vector<Flag>::iterator iter = flags.begin(); iter < flags.end(); iter++)
    {
        int8_t numBits = iter->getNumBits();

        // TODO: Confirm we're doing big-endian vs little-endian correctly
        uint64_t value = (buffer >> (startBitIdx - numBits + 1)) & ((0x1 << numBits) - 1);
        for (int8_t bitIdx = startBitIdx; bitIdx > startBitIdx - numBits; bitIdx--)
        {
          if (((buffer >> bitIdx) & 0x1) == 0x1)
          { out += "1"; }
          else
          { out += "0"; }
        }
        out += " (";
        out += iter->getInterpretation(value);
        out += ") ";

        startBitIdx -= numBits;
    }

    return out;
}

FlagsInterpretation::Flag::Flag(uint8_t numBits, initializer_list<string> flagValues): numBits(numBits), flagValues(flagValues) {}

FlagsInterpretation::Flag::Flag(uint8_t numBits, string flagValue): numBits(numBits)
{
    flagValues = vector<string>(1 << numBits, flagValue);
}

uint8_t FlagsInterpretation::Flag::getNumBits()
{
    return numBits;
}

string FlagsInterpretation::Flag::getInterpretation(unsigned int value)
{
    return flagValues.at(value);
}

ConditionalInterpretation::ConditionalInterpretation(Node* node, Interpretation* pDefault, initializer_list<Condition> conditions): node(node), pDefault(pDefault), conditions(conditions) {}

string ConditionalInterpretation::format(IByteIterator& data, Locale locale)
{
    IByteIterator* itr = node->dataNode->accessor->iterator();

    uint64_t nodeValue = IntInterpretation::readAs64Bits(*itr, IntInterpretation::OPT_LITTLE_ENDIAN);

    for (vector<Condition>::iterator condIter = conditions.begin(); condIter < conditions.end(); condIter++)
    {
        if (condIter->getValueMatch() == nodeValue)
        {
            return condIter->getpInterpretation()->format(data, locale);
        }
    }

    return pDefault->format(data, locale);
}

ConditionalInterpretation::Condition::Condition(uint64_t valueMatch, Interpretation* pInterpretation): valueMatch(valueMatch), pInterpretation(pInterpretation) {}

uint64_t ConditionalInterpretation::Condition::getValueMatch()
{
    return valueMatch;
}

Interpretation* ConditionalInterpretation::Condition::getpInterpretation()
{
    return pInterpretation;
}

EnumInterpretation::Enum::Enum(uint64_t valueMatch, string meaning): valueMatch(valueMatch), meaning(meaning) {}

uint64_t EnumInterpretation::Enum::getValueMatch()
{
    return valueMatch;
}

string EnumInterpretation::Enum::getMeaning()
{
    return meaning;
}

EnumInterpretation::EnumInterpretation(string defaultMeaning, uint32_t opts, initializer_list<Enum> enums): defaultMeaning(defaultMeaning), opts(opts), enums(enums) {}

string EnumInterpretation::format(IByteIterator& data, Locale locale)
{
    uint64_t nodeValue = IntInterpretation::readAs64Bits(data, opts);
    data.reset();

    IntInterpretation intInterp = IntInterpretation(opts);
    string out = intInterp.format(data, locale);
    out += " (";
    
    for (vector<Enum>::iterator enumIter = enums.begin(); enumIter < enums.end(); enumIter++)
    {
        if (enumIter->getValueMatch() == nodeValue)
        {
            out += enumIter->getMeaning();
            out += ")";
            return out;
        }
    }

    out += defaultMeaning;
    out += ")";
    return out;
}

Interpretation* Interpretation::asciz = new AscizInterpretation();
Interpretation* Interpretation::ascii = new AsciiInterpretation();
Interpretation* Interpretation::hex = new HexInterpretation();
Interpretation* Interpretation::msdosDate = new MsdosDateInterpretation();
Interpretation* Interpretation::msdosTime = new MsdosTimeInterpretation();
