#include "math.h"
#include <stdint.h>

double wrap_angle(double x)
{
    while (x > PI)
        x -= TWO_PI;
    while (x < -PI)
        x += TWO_PI;
    return x;
}

double sin(double x)
{
    return cos(PI / 2 - x);
}

double cos(double x)
{
    x = wrap_angle(x);

    double x2 = x * x;
    double result = 1.0;
    double term = 1.0;

    term *= -x2 / (1.0 * 2.0);
    result += term;

    term *= -x2 / (3.0 * 4.0);
    result += term;

    term *= -x2 / (5.0 * 6.0);
    result += term;

    term *= -x2 / (7.0 * 8.0);
    result += term;

    return result;
}

double fabs(double x)
{
    return x < 0 ? -x : x;
}

double floor(double x)
{
    int i = (int)x;
    return (x < 0 && x != i) ? i - 1 : i;
}

double ceil(double x)
{
    int i = (int)x;
    return (x > 0 && x != i) ? i + 1 : i;
}

double fmod(double x, double y)
{
    if (y == 0.0)
        return 0.0; // or NaN
    int n = (int)(x / y);
    return x - (double)n * y;
}

double pow(double base, double exp)
{
    if (exp == 0)
        return 1;
    if (exp == 1)
        return base;
    if (exp < 0)
        return 1.0 / pow(base, -exp);

    double result = 1.0;
    for (int i = 0; i < (int)exp; i++)
    {
        result *= base;
    }

    double frac = exp - (int)exp;
    if (frac > 0.0)
    {
        result *= 1 + frac * (base - 1);
    }

    return result;
}

double acos(double x)
{
    if (x < -1.0)
        x = -1.0;
    if (x > 1.0)
        x = 1.0;

    double negate = x < 0;
    x = fabs(x);

    double ret = -0.0187293;
    ret = ret * x + 0.0742610;
    ret = ret * x - 0.2121144;
    ret = ret * x + 1.5707288;
    ret = ret * sqrt(1.0 - x);
    return negate ? 3.14159265358979 - ret : ret;
}
double sqrt(double x)
{
    if (x <= 0.0)
        return 0.0;

    union
    {
        double d;
        uint64_t u;
    } val = {x};

    val.u = (val.u >> 1) + 0x1ff8000000000000ULL;

    double y = val.d;
    y = 0.5 * (y + x / y);
    y = 0.5 * (y + x / y);

    return y;
}