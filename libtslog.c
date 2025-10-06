// libtslog.c (VERSÃO CORRIGIDA E ROBUSTA)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include "libtslog.h"

static FILE *logfile = NULL;
static tslog_level_t current_level = TSLOG_INFO;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Converte enum para string (esta parte estava correta)
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

// --- FUNÇÃO CENTRAL CORRIGIDA ---
// Esta é a nova função "worker". Note que ela recebe 'va_list args'.
// O "v" em vlog é uma convenção para "variadic list".
void tslog_vlog(tslog_level_t level, const char *fmt, va_list args) {
    if (!logfile || level < current_level) return;

    pthread_mutex_lock(&lock);

    // Timestamp
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_buf[25];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", t);

    // Imprime o cabeçalho do log
    fprintf(logfile, "%s [%s] ", time_buf, level_to_str(level));

    // Usa vfprintf para imprimir a lista de argumentos formatada (a forma correta)
    vfprintf(logfile, fmt, args);

    fprintf(logfile, "\n");
    fflush(logfile);

    pthread_mutex_unlock(&lock);
}

// --- WRAPPERS (FUNÇÕES PÚBLICAS) CORRIGIDOS ---
// Agora, TODAS as funções públicas chamam a 'tslog_vlog'.

// A tslog_log original agora é apenas mais um wrapper.
void tslog_log(tslog_level_t level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    tslog_vlog(level, fmt, args);
    va_end(args);
}

void tslog_debug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    tslog_vlog(TSLOG_DEBUG, fmt, args);
    va_end(args);
}

void tslog_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    tslog_vlog(TSLOG_INFO, fmt, args);
    va_end(args);
}

void tslog_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    tslog_vlog(TSLOG_WARN, fmt, args);
    va_end(args);
}

void tslog_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    tslog_vlog(TSLOG_ERROR, fmt, args);
    va_end(args);
}

void tslog_fatal(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    tslog_vlog(TSLOG_FATAL, fmt, args);
    va_end(args);
}