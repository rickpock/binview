#ifndef BINVIEW_BYTE_ITERATOR
#define BINVIEW_BYTE_ITERATOR

typedef unsigned char byte;

/* Note: I've made a valiant attempt to implement these as std::iterator's, 
 *       but it was just burning hours. At its crux, std::iterator's require
 *       the == operator to be overloaded, which is implementation-specific
 *       (different for ByteIterator over memory vs over file contents vs
 *       aggregating other iterators) and I couldn't get C++ to generalize them.
 *
 *       In contrast, this implementation took me five minutes.
 */

template<class T>
class IIterator
{
public:
    virtual T next() = 0;
    virtual bool hasNext() = 0;
};

typedef IIterator<byte> ByteIterator;

class MemoryIterator : ByteIterator
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

#endif
