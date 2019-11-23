// EPOS OStream Interface

#include <system/config.h>

#ifndef __ostream_h
#define __ostream_h

extern "C" {
    void _print_preamble();
    void _print(const char * s);
    void _print_trailler(bool error);
}

__BEGIN_UTIL

class OStream
{
public:
    struct Begl {};
    struct Endl {};
    struct Hex {};
    struct Dec {};
    struct Oct {};
    struct Bin {};
    struct Err {};

public:
    OStream(): _base(10), _error(false) {}

    OStream & operator<<(const Begl & begl) {
        if(Traits<System>::multicore)
            _print_preamble();
        return *this;
    }

    OStream & operator<<(const Endl & endl) {
        if(Traits<System>::multicore)
            _print_trailler(_error);
        print("\n");
        _base = 10;
        return *this;
    }

    OStream & operator<<(const Hex & hex) {
        _base = 16;
        return *this;
    }
    OStream & operator<<(const Dec & dec) {
        _base = 10;
        return *this;
    }
    OStream & operator<<(const Oct & oct) {
        _base = 8;
        return *this;
    }
    OStream & operator<<(const Bin & bin) {
        _base = 2;
        return *this;
    }

    OStream & operator<<(const Err & err)
    {
        _error = true;
        return *this;
    }

    OStream & operator<<(char c) {
        char buf[2];
        buf[0] = c;
        buf[1] = '\0';
        print(buf);
        return *this;
    }
    OStream & operator<<(unsigned char c) {
        return operator<<(static_cast<unsigned int>(c));
    }

    OStream & operator<<(int i) {
        char buf[64];
        buf[itoa(i, buf)] = '\0';
        print(buf);
        return *this;
    }
    OStream & operator<<(short s) {
        return operator<<(static_cast<int>(s));
    }
    OStream & operator<<(long l) {
        return operator<<(static_cast<int>(l));
    }

    OStream & operator<<(unsigned int u) {
        char buf[64];
        buf[utoa(u, buf)] = '\0';
        print(buf);
        return *this;
    }
    OStream & operator<<(unsigned short s) {
        return operator<<(static_cast<unsigned int>(s));
    }
    OStream & operator<<(unsigned long l) {
        return operator<<(static_cast<unsigned int>(l));
    }

    OStream & operator<<(long long int u) {
        char buf[64];
        buf[llitoa(u, buf)] = '\0';
        print(buf);
        return *this;
    }

    OStream & operator<<(unsigned long long int u) {
        char buf[64];
        buf[llutoa(u, buf)] = '\0';
        print(buf);
        return *this;
    }

    OStream & operator<<(const void * p) {
        char buf[64];
        buf[ptoa(p, buf)] = '\0';
        print(buf);
        return *this;
    }

    OStream & operator<<(const char * s) {
        print(s);
        return *this;
    }

    OStream & operator<<(float f) {
        if(f < 0.0001f && f > -0.0001f)
            (*this) << "0.0000";

        int b = 0;
        int m = 0;

        float x = f;
        if(x >= 0.0001f) {
            while(x >= 1.0000f) {
                x -= 1.0f;
                b++;
            }
            (*this) << b << ".";
            for(int i = 0; i < 3; i++) {
                m = 0;
                x *= 10.0f;
                while(x >= 1.000f) {
                    x -= 1.0f;
                    m++;
                }
                (*this) << m;
            }
        } else {
            while(x <= -1.000f) {
                x += 1.0f;
                b++;
            }
            (*this) << "-" << b << ".";
            for(int i = 0; i < 3; i++) {
                m = 0;
                x *= 10.0f;
                while(x <= -1.000f) {
                    x += 1.0f;
                    m++;
                }
                (*this) << m;
            }
        }
        return *this;
    }

    OStream & operator<<(double d) {
        return operator<<(static_cast<float>(d));
    }

    // src: http://beedub.com/Sprite093/src/lib/c/stdlib/atof.c
    /*
    *----------------------------------------------------------------------
    *
    * atof --
    *
    *	This procedure converts a floating-point number from an ASCII
    *	decimal representation to internal double-precision format.
    *
    * Results:
    *	The return value is the floating-point equivalent of string.
    *	If a terminating character is found before any floating-point
    *	digits, then zero is returned.
    *
    * Side effects:
    *	None.
    *
    *----------------------------------------------------------------------
    */
    double atof(char *string)
                    /* A decimal ASCII floating-point number,
                    * optionally preceded by white space.
                    * Must have form "-I.FE-X", where I is the
                    * integer part of the mantissa, F is the
                    * fractional part of the mantissa, and X
                    * is the exponent.  Either of the signs
                    * may be "+", "-", or omitted.  Either I
                    * or F may be omitted, or both.  The decimal
                    * point isn't necessary unless F is present.
                    * The "E" may actually be an "e".  E and X
                    * may both be omitted (but not just one).
                    */
    {
        double powersOf10[] = {	/* Table giving binary powers of 10.  Entry */
            10.,			    /* is 10^2^i.  Used to convert decimal */
            100.,			    /* exponents into floating-point numbers. */
            1.0e4,
            1.0e8,
            1.0e16,
            1.0e32,
            1.0e64,
            1.0e128,
            1.0e256
        };

        int sign, expSign = 0;
        double fraction, dblExp, *d;
        register char *p, c;
        int exp = 0;		/* Exponent read from "EX" field. */
        int fracExp = 0;		/* Exponent that derives from the fractional
                    * part.  Under normal circumstatnces, it is
                    * the negative of the number of digits in F.
                    * However, if I is very long, the last digits
                    * of I get dropped (otherwise a long I with a
                    * large negative exponent could cause an
                    * unnecessary overflow on I alone).  In this
                    * case, fracExp is incremented one for each
                    * dropped digit.
                    */
        int mantSize;		/* Number of digits in mantissa. */
        int decPt;			/* Number of mantissa digits BEFORE decimal
                    * point.
                    */
        char *pExp;			/* Temporarily holds location of exponent
                    * in string.
                    */

        /*
        * Strip off leading blanks and check for a sign.
        */

        p = string;
        while (isspace(*p)) {
            p += 1;
        }
        if (*p == '-') {
            sign = 	1;
            p += 1;
        } else {
            if (*p == '+') {
                p += 1;
            }
            sign = 0;
        }

        /*
        * Count the number of digits in the mantissa (including the decimal
        * point), and also locate the decimal point.
        */

        decPt = -1;
        for (mantSize = 0; ; mantSize += 1)
        {
            c = *p;
            if (!isdigit(c)) {
                if ((c != '.') || (decPt >= 0)) {
                    break;
                }
                decPt = mantSize;
            }
            p += 1;
        }

        /*
        * Now suck up the digits in the mantissa.  Use two integers to
        * collect 9 digits each (this is faster than using floating-point).
        * If the mantissa has more than 18 digits, ignore the extras, since
        * they can't affect the value anyway.
        */

        pExp  = p;
        p -= mantSize;
        if (decPt < 0) {
            decPt = mantSize;
        } else {
            mantSize -= 1;			/* One of the digits was the point. */
        }
        if (mantSize > 18) {
            fracExp = decPt - 18;
            mantSize = 18;
        } else {
            fracExp = decPt - mantSize;
        }
        if (mantSize == 0) {
            return 0.0;
        } else {
            int frac1, frac2;
            frac1 = 0;
            for ( ; mantSize > 9; mantSize -= 1)
            {
                c = *p;
                p += 1;
                if (c == '.') {
                c = *p;
                p += 1;
                }
                frac1 = 10*frac1 + (c - '0');
            }
            frac2 = 0;
            for (; mantSize > 0; mantSize -= 1)
            {
                c = *p;
                p += 1;
                if (c == '.') {
                c = *p;
                p += 1;
                }
                frac2 = 10*frac2 + (c - '0');
            }
            fraction = (1.0e9 * frac1) + frac2;
        }

        /*
        * Skim off the exponent.
        */

        p = pExp;
        if ((*p == 'E') || (*p == 'e')) {
            p += 1;
            if (*p == '-') {
                expSign = 1;
                p += 1;
            } else {
                if (*p == '+') {
                p += 1;
                }
                expSign = 0;
            }
            while (isdigit(*p)) {
                exp = exp * 10 + (*p - '0');
                p += 1;
            }
        }
        if (expSign) {
            exp = fracExp - exp;
        } else {
            exp = fracExp + exp;
        }

        /*
        * Generate a floating-point number that represents the exponent.
        * Do this by processing the exponent one bit at a time to combine
        * many powers of 2 of 10. Then combine the exponent with the
        * fraction.
        */

        if (exp < 0) {
            expSign = 1;
            exp = -exp;
        } else {
            expSign = 0;
        }
        if (exp > maxExponent) {
            exp = maxExponent;
        }
        dblExp = 1.0;
        for (d = powersOf10; exp != 0; exp >>= 1, d += 1) {
            if (exp & 01) {
                dblExp *= *d;
            }
        }
        if (expSign) {
            fraction /= dblExp;
        } else {
            fraction *= dblExp;
        }

        if (sign) {
            return -fraction;
        }
        return fraction;
    }

// src: http://beedub.com/Sprite093/src/lib/c/stdlib/atof.c
private:
    static const int maxExponent = 511;	/* Largest possible base 10 exponent.  Any
                                     * exponent larger than this will already
				                     * produce underflow or overflow, so there's
				                     * no need to worry about additional digits.
				                     */

private:
    void print(const char * s) { _print(s); }

    int itoa(int v, char * s);
    int utoa(unsigned int v, char * s, unsigned int i = 0);
    int llitoa(long long int v, char * s);
    int llutoa(unsigned long long int v, char * s, unsigned int i = 0);
    int ptoa(const void * p, char * s);
    bool isspace(unsigned char c);
    bool isdigit(unsigned char c);

private:
    int _base;
    volatile bool _error;

    static const char _digits[];
};

extern OStream::Begl begl;
extern OStream::Endl endl;
extern OStream::Hex hex;
extern OStream::Dec dec;
extern OStream::Oct oct;
extern OStream::Bin bin;

__END_UTIL

__BEGIN_SYS
extern OStream kout, kerr;
__END_SYS

#endif
