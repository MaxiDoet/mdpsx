#include <stdio.h>
#include <stdarg.h>

#include "log.h"

void log_info(char *prefix, const char *format, ...)
{
    va_list args;
    
    fprintf(stdout, "\e[0;32m%s\e[0m ", prefix);
    
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

void log_error(char *prefix, const char *format, ...)
{
    va_list args;
        
    fprintf(stdout, "\e[0;31m%s\e[0m ", prefix);        

    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

void log_debug(char *prefix, const char *format, ...)
{
    va_list args;
    
    fprintf(stdout, "\e[0;34m%s\e[0m ", prefix);

    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}