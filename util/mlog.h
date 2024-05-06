#ifndef H_LOG_151113
#define H_LOG_151113

#include <stdio.h>

/** @brief minimalist log utility
 *
 *  The idea is to have a few macro to have an easy way of logging and
 *  deactivate the log if need be. They are 4 logging macros that can be
 *  disabled or enabled depanding on the value of LOG_ENABLE:
 *
 *  - LOG_PRINT(stream, format, ...) This is just a wrapper around fprintf.
 *  This macro is enabled if LOG_ENABLE is defined
 *
 *  - LOG_ERROR(format, ...) This prints the format given to it as a parameter
 *  in the log_error_stream preceded by "[error] ".
 *  This macro is enabled if LOG_ENABLE is defined and its value is >= 2
 *
 *  - LOG_WARN(format, ...) This prints the format given to it as a parameter
 *  in the log_warn_stream preceded by "[warn] ".
 *  This macro is enabled if LOG_ENABLE is defined and its value is >= 3
 *
 *  - LOG_INFO(format, ...) This prints the format given to it as a parameter
 *  in the log_info_stream preceded by "[info] ".
 *  This macro is enabled if LOG_ENABLE is defined and its value is >= 4
 *
 *
 *
 *  By default, the log stream is stdout for info and warn, stderr for error.
 *  This can be changed by setting the log_xxxx_stream global variables
 *
 *  If the color.h header is included before log.h and the output stream for
 *  the log is stdout, some color will be added to the log output.
 *  
 */
 
extern FILE  *log_info_stream;
extern FILE  *log_warn_stream;
extern FILE  *log_error_stream;

#ifdef LOG_ENABLE

#ifdef COLOR_HAS_COLOR_FOR_THE_TERM
    #define LOG_setColor(x) setColor(x)
#else
    #define LOG_setColor(x)
#endif

#define LOG_PRINT(stream, format, ...) fprintf(stream, format, ##__VA_ARGS__)

    #if LOG_ENABLE >= 2
        #define LOG_ERROR(format, ...) do {                                     \
            putc('[', log_error_stream);                                        \
            LOG_setColor (RED);                                                 \
            fputs("error", log_error_stream );                                  \
            LOG_setColor (DEFAULT);                                             \
            LOG_PRINT(log_error_stream, "] " format, ##__VA_ARGS__);\
        } while(0)
    #else
        #define LOG_ERROR(format, ...)
    #endif
    
    #if LOG_ENABLE >= 3
        #define LOG_WARN(format, ...)   do {                  \
            putc('[', log_warn_stream);                       \
            LOG_setColor (YELLOW);                            \
            fputs( "warn", log_warn_stream);                  \
            LOG_setColor (DEFAULT);                                             \
            LOG_PRINT(log_warn_stream, "] " format, ##__VA_ARGS__);\
            } while(0)
    #else
        #define LOG_WARN(format, ...)
    #endif

    #if LOG_ENABLE >= 4
         #define LOG_INFO(format, ...)   do {                 \
            putc('[', log_info_stream);                       \
            LOG_setColor (GREEN);                             \
            fputs("info", log_info_stream);                   \
            LOG_setColor (DEFAULT);                                             \
            LOG_PRINT(log_info_stream, "] " format, ##__VA_ARGS__);\
            } while(0)
    #else
         #define LOG_INFO(format, ...)
    #endif
    
#else 
    #define LOG_PRINT(stream, format, ...)
    #define LOG_INFO(format, ...)
    #define LOG_WARN(format, ...)
    #define LOG_ERROR(format, ...)

#endif

#endif // H_LOG_151113
