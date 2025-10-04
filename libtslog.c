#include "libtslog.h"
#include <stdarg.h>
#include <time.h>
#include <pthread.h>

static FILE *logfile = NULL;
static tslog_level_t current_level = TSLOG_INFO;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Converte enum para string
static const char* level_to_str(tslog_level_t level) {
    switch (level) {
        case TSLOG_DEBUG: return "DEBUG";
        case TSLOG_INFO:  return "INFO";
        case TSLOG_WARN:  return "WARN";
        case TSLOG_ERROR: return "ERROR";
        case TSLOG_FATAL: return "FATAL";
        default:          return "UNKNOWN";
    }
}

int tslog_init(const char *filename, tslog_level_t level) {
    logfile = fopen(filename, "a");
    if (!logfile) return -1;
    current_level = level;
    return 0;
}

void tslog_close(void) {
    if (logfile) {
        fclose(logfile);
        logfile = NULL;
    }
}

void tslog_log(tslog_level_t level, const char *fmt, ...) {
    if (!logfile || level < current_level) return;

    pthread_mutex_lock(&lock);

    // timestamp
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(logfile, "%04d-%02d-%02d %02d:%02d:%02d [%s] ",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec,
        level_to_str(level));

    va_list args;
    va_start(args, fmt);
    vfprintf(logfile, fmt, args);
    va_end(args);

    fprintf(logfile, "\n");
    fflush(logfile);

    pthread_mutex_unlock(&lock);
}

// Wrappers
void tslog_debug(const char *fmt, ...) { va_list args; va_start(args, fmt); tslog_log(TSLOG_DEBUG, fmt, args); va_end(args); }
void tslog_info(const char *fmt, ...)  { va_list args; va_start(args, fmt); tslog_log(TSLOG_INFO,  fmt, args); va_end(args); }
void tslog_warn(const char *fmt, ...)  { va_list args; va_start(args, fmt); tslog_log(TSLOG_WARN,  fmt, args); va_end(args); }
void tslog_error(const char *fmt, ...) { va_list args; va_start(args, fmt); tslog_log(TSLOG_ERROR, fmt, args); va_end(args); }
void tslog_fatal(const char *fmt, ...) { va_list args; va_start(args, fmt); tslog_log(TSLOG_FATAL, fmt, args); va_end(args); }
