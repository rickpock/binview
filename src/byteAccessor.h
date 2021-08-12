#ifndef BINVIEW_BYTE_ACCESSOR
#define BINVIEW_BYTE_ACCESSOR

#include "byteIterator.h"

class IByteAccessor
{
public:
    virtual byte& operator[](long) = 0;
    virtual long getSize() = 0;
    virtual IByteAccessor* subset(long, long) = 0;
    virtual IByteIterator* iterator() = 0;
};

class MemoryAccessor : public IByteAccessor
{
private:
    byte* src;
    long len;

public:
    MemoryAccessor(byte* src, long len);

    byte& operator[](long);
    long getSize();
    IByteAccessor* subset(long, long);
    IByteIterator* iterator();
};

#endif
