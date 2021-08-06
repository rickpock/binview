#ifndef BINVIEW_INTERPRETATION
#define BINVIEW_INTERPRETATION

#include <string>

using namespace std;

enum Locale
{
    LOCALE_EN_US
};

class Interpretation
{
public:
    string (*format)(const unsigned char*, Locale);

    static void init();

private:
    Interpretation();
    Interpretation(string (*formatFunc)(const unsigned char*, Locale));
    static string formatAsciiz(const unsigned char* data, Locale locale);

public:
    static Interpretation asciiz;
};

#endif
