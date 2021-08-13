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


AggAccessor::AggAccessor(IByteAccessor* src[], int len)
{
    this->src = (IByteAccessor **)malloc(sizeof(IByteAccessor*) * len);
    for (int srcIdx = 0; srcIdx < len; srcIdx++)
    {
        this->src[srcIdx] = src[srcIdx];
    }

    this->len = len;
}

AggAccessor::~AggAccessor()
{
    free(src);
}

byte& AggAccessor::operator[](long idx)
{
    int srcIdx;
    long offsetIdx;
    bool result = srcIdxFromByteIdx(idx, srcIdx, offsetIdx);
    // TODO: Handle out-of-bounds
    
    return (*src[srcIdx])[offsetIdx];
}

long AggAccessor::getSize()
{
    long totalLen = 0;
    for (int srcIdx = 0; srcIdx < len; srcIdx++)
    {
        totalLen += src[srcIdx]->getSize();
    }
    return totalLen;
}

IByteAccessor* AggAccessor::subset(long startIdx, long len)
{
    int startSrcIdx;
    long startOffsetIdx;
    bool result = srcIdxFromByteIdx(startIdx, startSrcIdx, startOffsetIdx);

    int endSrcIdx;
    long endOffsetIdx;
    result = srcIdxFromByteIdx(startIdx + len - 1, endSrcIdx, endOffsetIdx);

    int subsetLen = endSrcIdx - startSrcIdx;

    IByteAccessor** subsetArr = (IByteAccessor **)malloc(sizeof(IByteAccessor*) * subsetLen);
    subsetArr[0] = src[startSrcIdx]->subset(startIdx, src[startSrcIdx]->getSize() - startIdx);
    subsetArr[subsetLen - 1] = src[endSrcIdx]->subset(0, endOffsetIdx);

    for (int srcIdx = 1; srcIdx < (subsetLen - 1); srcIdx++)
    {
        subsetArr[srcIdx] = src[srcIdx];
    }

    return new AggAccessor(subsetArr, subsetLen);
}

IByteIterator* AggAccessor::iterator()
{
    // TODO
}

bool AggAccessor::srcIdxFromByteIdx(long byteIdx, int& srcIdx, long& offsetIdx)
{
    if (byteIdx < 0)
    { return false; }

    srcIdx = 0;
    while (byteIdx >= src[srcIdx]->getSize())
    {
        byteIdx -= src[srcIdx]->getSize();
        srcIdx++;

        if (srcIdx >= len)
        { return false; }
    }

    offsetIdx = byteIdx;
    return true;
}
