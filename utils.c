#include <stdio.h>
#include <stdarg.h>
#include "utils.h"

void debuglog(int one_in_n_chance, const char* format, ...)
{
    if (!debuglog) {
        return;
    }
    if (rand() % one_in_n_chance == 0) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}
