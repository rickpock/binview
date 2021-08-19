#include "byteAccessor.h"

MemoryAccessor::MemoryAccessor(byte* src, long len) : src(src), len(len) {}

byte MemoryAccessor::operator[](long idx)
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

byte AggAccessor::operator[](long idx)
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

    AggAccessor* out = new AggAccessor(subsetArr, subsetLen);
    free(subsetArr);
    return out;
}

IByteIterator* AggAccessor::iterator()
{
    IByteIterator** itrArr = (IByteIterator**)malloc(sizeof(IByteIterator*) * len);
    for (int srcIdx = 0; srcIdx < len; srcIdx++)
    {
        itrArr[srcIdx] = src[srcIdx]->iterator();
    }

    AggIterator* out = new AggIterator(itrArr, len);
    free(itrArr);
    return out;
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

FileAccessor::FileAccessor(FILE *fp, long offset, long len, bool owner) : fp(fp), owner(owner), offset(offset), len(len) {}

FileAccessor::FileAccessor(FILE *fp, bool owner) : FileAccessor(fp, 0L, -1L, owner) {}

byte FileAccessor::operator[](long loc)
{
    long origPos;
    if (! owner)
    {
        origPos = ftell(fp);
        if (fseek(fp, loc, SEEK_SET) != 0)
        {
            // TODO: Error handling
        }
    }

    if (feof(fp))
    {
        // TODO
    }

    byte out = fgetc(fp);

    if (! owner)
    {
        fseek(fp, origPos, SEEK_SET); // TODO: Error handling
    }

    return out;
}

long FileAccessor::getSize()
{
    if (len != -1)
    { return len; }

    if (fileSize != -1L)
    { return fileSize; }

    long origPos = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, origPos, SEEK_SET);

    return fileSize;    
}

IByteAccessor* FileAccessor::subset(long offset, long len)
{
    return new FileAccessor(fp, this->offset + offset, len, false);
}

IByteIterator* FileAccessor::iterator()
{
    return new FileIterator<1024>(fp, offset, len, false);
}
