#include "byteAccessor.h"

MemoryAccessor::MemoryAccessor(byte* src, long len) : src(src), len(len) {}

byte& MemoryAccessor::operator[](long idx)
{
    return src[idx];
}

long MemoryAccessor::getSize()
{
    return len;
}

IByteAccessor* MemoryAccessor::subset(long startIdx, long len)
{
    // TODO: Error handling
    return new MemoryAccessor(&(src[startIdx]), len);
}

IByteIterator* MemoryAccessor::iterator()
{
    return new MemoryIterator(src, len);
}
