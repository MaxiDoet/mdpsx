#ifndef _log_h
#define _log_h

#include <stdint.h>

#define LOG_LEVEL_INFO  0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_DEBUG 2

#define LOG_DEBUG_R3000
#define LOG_DEBUG_R3000_EXCEPTIONS
//#define LOG_DEBUG_BUS_READ
//#define LOG_DEBUG_BUS_WRITE
#define LOG_DEBUG_BUS_WRITE_IO

void log_info(char *prefix, const char *format, ...);
void log_error(char *prefix, const char *format, ...);
void log_debug(char *prefix, const char *format, ...);

#endif