#include "byteIterator.h"

MemoryIterator::MemoryIterator(byte* src, long len) : _src(src), curr(src), _len(len) {}

byte MemoryIterator::next()
{
    byte* b = curr;
    curr++;
    return *b;
}

bool MemoryIterator::hasNext()
{
    return _src + _len > curr;
}

void MemoryIterator::reset()
{
    curr = _src;
}
