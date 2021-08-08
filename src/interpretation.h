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
    string (*format)(const byte*, Locale);

    static void init();

private:
    Interpretation();
    Interpretation(string (*formatFunc)(const byte*, Locale));
    static string formatAsciz(const byte* data, Locale locale);

public:
    static Interpretation asciz;
};

#endif
