#include "math.h"

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
