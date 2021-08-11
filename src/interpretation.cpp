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

IntInterpretation::IntInterpretation(int opts) : opts(opts) {}

bool IntInterpretation::isSystemLittleEndian()
{
    short test = 1;
    return *(char *)&test;
}

// TODO
// WARNING: This interpretation only works up to "long" size
string IntInterpretation::format(IByteIterator& data, Locale locale)
{
    string out = "";

    unsigned long value = 0;

    // Read source from least-significant byte to most significant byte
    // Write into the output memory from least significant byte to most significant byte
    if ((opts & OPT_MASK_ENDIAN) == OPT_LITTLE_ENDIAN)
    {
        // Interpret data as Little Endian
        if (isSystemLittleEndian())
        {
            // Read source from left to right
            // Write into the long from left to right
            int longIdx = 0;
            while (data.hasNext() && longIdx < sizeof(unsigned long))
            {
                ((char *)&value)[longIdx] = data.next();
                longIdx++;
            }
        } else {
            // Read source from left to right
            // Write into the long from right to left
            int longIdx = sizeof(unsigned long);
            while (data.hasNext() && longIdx >= 0)
            {
                ((char *)&value)[longIdx] = data.next();
                longIdx--;
            }
        }

        out = std::to_string(value);
	
        // Print hex right to left when little endian
        if ((opts & OPT_MASK_HEX) == OPT_INCL_HEX)
        {
            byte buffer[sizeof(unsigned long)];
            for (int bufferIdx = 0; bufferIdx < sizeof(unsigned long); bufferIdx++)
            {
                buffer[bufferIdx] = 0;
            }

            int bufferIdx = 0;
            int dataLen = 0;

            data.reset();
            while (data.hasNext())
            {
                buffer[bufferIdx] = data.next();
                bufferIdx = (bufferIdx + 1) % sizeof(unsigned long);
                dataLen++;
            }

            out = out + " (0x";
    
            char charBuffer[3];
            for (int longIdx = 0; longIdx < sizeof(unsigned long) && longIdx < dataLen; longIdx++)
            {
                // Read from buffer backwards (right to left), starting from (bufferIdx - 1)
                bufferIdx = (bufferIdx - 1 + sizeof(unsigned long)) % sizeof(unsigned long);

                sprintf(charBuffer, "%02X", buffer[bufferIdx]);
                out += charBuffer;
            }
    
            out = out + ")";
        }
    } else {
        // Interpret data as Big Endian

        // Read source from right to left
        byte buffer[sizeof(unsigned long)];
        for (int bufferIdx = 0; bufferIdx < sizeof(unsigned long); bufferIdx++)
        {
            buffer[bufferIdx] = 0;
        }

        int bufferIdx = 0;
        while (data.hasNext())
        {
            buffer[bufferIdx] = data.next();
            bufferIdx = (bufferIdx + 1) % sizeof(unsigned long);
        }

        if (isSystemLittleEndian())
        {
            // Write into the long from left to right
            for (int longIdx = 0; longIdx < sizeof(unsigned long); longIdx++)
            {
                // Read from buffer backwards (right to left), starting from (bufferIdx - 1)
                bufferIdx = (bufferIdx - 1 + sizeof(unsigned long)) % sizeof(unsigned long);

                ((char *)&value)[longIdx] = buffer[bufferIdx];
            }
        } else {
            // Write into the long from right to left
            for (int longIdx = sizeof(unsigned long); longIdx > 0; longIdx--)
            {
                // Read from buffer backwards (right to left), starting from (bufferIdx - 1)
                bufferIdx = (bufferIdx - 1 + sizeof(unsigned long)) % sizeof(unsigned long);

                ((char *)&value)[longIdx] = buffer[bufferIdx];
            }
        }

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

AscizInterpretation Interpretation::asciz = AscizInterpretation();
AsciiInterpretation Interpretation::ascii = AsciiInterpretation();
HexInterpretation Interpretation::hex = HexInterpretation();
