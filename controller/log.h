#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LOG_LEVEL_QUIET = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,

    LOG_LEVEL_DEFAULT = LOG_LEVEL_INFO,
    LOG_LEVEL_MIN = LOG_LEVEL_QUIET,
    LOG_LEVEL_MAX = LOG_LEVEL_DEBUG
} LogLevel;

#define log_e(...)    log_printf(LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_w(...)    log_printf(LOG_LEVEL_WARNING, __VA_ARGS__)
#define log_i(...)    log_printf(LOG_LEVEL_INFO, __VA_ARGS__)
#define log_d(...)    log_printf(LOG_LEVEL_DEBUG, __VA_ARGS__)

void log_open(char *program_name, char *log_output, LogLevel level);
void log_close(void);
void log_set_level(LogLevel level);
void log_printf(LogLevel level, char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
