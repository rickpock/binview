#ifndef BINVIEW_BYTE_ITERATOR
#define BINVIEW_BYTE_ITERATOR

typedef unsigned char byte;

/* Note: I've made a valiant attempt to implement these as std::iterator's, 
 *       but it was just burning hours. At its crux, std::iterator's require
 *       the == operator to be overloaded, which is implementation-specific
 *       (different for IByteIterator over memory vs over file contents vs
 *       aggregating other iterators) and I couldn't get C++ to generalize them.
 *
 *       In contrast, this implementation took me five minutes.
 *
 *       We're sticking with this implementation.
 */

template<class T>
class IIterator
{
public:
    virtual T next() = 0;
    virtual bool hasNext() = 0;
};

typedef IIterator<byte> IByteIterator;

class MemoryIterator : public IByteIterator
{
private:
    byte* _src;
    long _len;

    byte* curr;

public:
    MemoryIterator(byte* src, long len);

    byte next();

    bool hasNext();
};

#include <stdio.h>

template<unsigned long BUFFER_SZ>
class FileIterator : public IByteIterator
{
private:
    FILE* _fp;

    bool _owner; // If this iterator does not own the file pointer, we need to reset the seek position after reading.

    long loc;    // Current location in the file. Used to seek if this iterator does not own the file pointer.
    long end;

    byte buffer[BUFFER_SZ];
    long bufferOffset;
    long bufferBytes;

    void fillBufferIfNeeded()
    {
        if (bufferOffset == BUFFER_SZ)
        {
            long origPos;
            if (! _owner)
            {
                origPos = ftell(_fp);
                if (fseek(_fp, loc, SEEK_SET) != 0)
                {
                    // TODO: Error handling
                }
            }

            // fetch next BUFFER_SZ bytes
            // bufferBytes is set to number of bytes actually read into buffer
            for (bufferBytes = 0; bufferBytes < BUFFER_SZ; bufferBytes++)
            {
                // check for end of file
                if (feof(_fp))
                { break; }

                // check for end of specified section
                if (end != -1 && loc + bufferBytes == end)
                { break; }

                buffer[bufferBytes] = fgetc(_fp);
            }

            if (! _owner)
            {
                if (fseek(_fp, loc, SEEK_SET) != 0)
                {
                    // TODO: Error handling
                }
            }

            bufferOffset = 0;
        }
    }

public:
    FileIterator(FILE *fp, long offset, long len, bool owner = false) : _fp(fp), _owner(owner), loc(offset), end(len == -1 ? -1 : offset + len), bufferOffset(BUFFER_SZ) {};
    FileIterator(FILE *fp, bool owner = false) : FileIterator(fp, 0L, -1L, owner) {}
    /* Note: If owner is set to false and no offset is specified, it will read from the beginning of the file
     *       If owner is set to true and no offset is specified, it will read from wherever fp was left
     */

    byte next()
    {
        fillBufferIfNeeded();

        byte b = buffer[bufferOffset];

        bufferOffset++;
        loc++;

        return b;
    }

    bool hasNext()
    {
        fillBufferIfNeeded();

        return bufferOffset < bufferBytes;
    }
};

// TODO: AggByteIterator

#endif
