#ifndef LIBTSLOG_H
#define LIBTSLOG_H

#include <stdio.h>

// Níveis de log
typedef enum {
    TSLOG_DEBUG,
    TSLOG_INFO,
    TSLOG_WARN,
    TSLOG_ERROR,
    TSLOG_FATAL
} tslog_level_t;

// Inicializa logger (arquivo de log + nível mínimo)
int tslog_init(const char *filename, tslog_level_t level);

// Fecha logger
void tslog_close(void);

// Função principal de log (com formatação estilo printf)
void tslog_log(tslog_level_t level, const char *fmt, ...);

// Wrappers para cada nível
void tslog_debug(const char *fmt, ...);
void tslog_info(const char *fmt, ...);
void tslog_warn(const char *fmt, ...);
void tslog_error(const char *fmt, ...);
void tslog_fatal(const char *fmt, ...);

#endif
