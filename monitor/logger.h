#ifndef LOGGER_H
#define LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LOGGER_LEVEL_QUIET = -1,
    LOGGER_LEVEL_PANIC,
    LOGGER_LEVEL_FATAL,
    LOGGER_LEVEL_ERROR,
    LOGGER_LEVEL_WARNING,
    LOGGER_LEVEL_INFO,
    LOGGER_LEVEL_VERBOSE,
    LOGGER_LEVEL_DEBUG,
    LOGGER_LEVEL_TRACE,

    LOGGER_LEVEL_DEFAULT = LOGGER_LEVEL_WARNING,
    LOGGER_LEVEL_MIN = LOGGER_LEVEL_QUIET,
    LOGGER_LEVEL_MAX = LOGGER_LEVEL_TRACE
} LoggerLevel;

typedef struct LoggerContext {
    FILE *fp;
    int level;
} LoggerContext;

int logger_init(char *name, LoggerContext *context);
void logger_uninit(LoggerContext *context);
int logger_set_level(LoggerContext *context, LoggerLevel level);
int logger_printf(LoggerContext *context, LoggerLevel level, char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
