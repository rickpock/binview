#ifndef BINVIEW_INTERPRETATION
#define BINVIEW_INTERPRETATION

#include <string>
#include "byteIterator.h"

using namespace std;

enum Locale
{
    LOCALE_EN_US
};

class Interpretation
{
public:
    string (*format)(IByteIterator&, Locale);

    static void init();

private:
    Interpretation();
    Interpretation(string (*formatFunc)(IByteIterator&, Locale));
    static string formatAsciz(IByteIterator& data, Locale locale);

public:
    static Interpretation asciz;
};

#endif
