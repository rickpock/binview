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

AggIterator::AggIterator(IByteIterator* src[], int len)
{
    this->src = (IByteIterator**)malloc(sizeof(IByteIterator*) * len);
    for (int srcIdx = 0; srcIdx < len; srcIdx++)
    {
        this->src[srcIdx] = src[srcIdx];
    }

    this->len = len;
}

AggIterator::~AggIterator()
{
    free(src);
}

// Moves currIdx ahead until it points to an iterator that is non-empty OR reaches the last iterator
void AggIterator::advanceToNonEmptyIterator()
{
    while (currIdx < len && (! src[currIdx]->hasNext()))
    { currIdx++; }
}

byte AggIterator::next()
{
    advanceToNonEmptyIterator();

    return src[currIdx]->next();
}

bool AggIterator::hasNext()
{
    advanceToNonEmptyIterator();

    return src[currIdx]->hasNext();
}

void AggIterator::reset()
{
    for (int srcIdx = 0; srcIdx < len; srcIdx++)
    {
        this->src[srcIdx]->reset();
    }
}
