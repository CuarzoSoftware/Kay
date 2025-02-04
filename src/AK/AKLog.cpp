#include <AK/AKLog.h>
#include <cstdlib>
#include <stdio.h>
#include <stdarg.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define BRELN "\n"

static int level = 0;

using namespace AK;

void AKLog::init()
{
    char *env = getenv("KAY_DEBUG");

    if (env)
        level = atoi(env);
    else
        level = 0;
}

void AKLog::fatal(const char *format, ...)
{
    if (level >= 1)
    {
        fprintf(stderr, "%sKay fatal:%s ", KRED, KNRM);
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        fprintf(stderr, BRELN);
    }
}

void AKLog::error(const char *format, ...)
{
    if (level >= 2)
    {
        fprintf(stderr, "%sKay error:%s ", KRED, KNRM);
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        fprintf(stderr, BRELN);
    }
}

void AKLog::warning(const char *format, ...)
{
    if (level >= 3)
    {
        printf("%sKay warning:%s ", KYEL, KNRM);
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf(BRELN);
    }
}

void AKLog::debug(const char *format, ...)
{
    if (level >= 4)
    {
        printf("%sKay debug:%s ", KGRN, KNRM);
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf(BRELN);
    }
}

void AKLog::log(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf(BRELN);
}
