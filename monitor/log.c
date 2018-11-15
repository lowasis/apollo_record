#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>
#include <sys/time.h>
#include "log.h"


#define HOST_NAME_LENGTH        32
#define LEVEL_STR_LENGTH        8


static char config_host_name[HOST_NAME_LENGTH + 1];
static char *config_program_name;
static int config_output_syslog;
static LogLevel config_level;


static int level_to_syslog_level(LogLevel level)
{
    switch (level)
    {
        case LOG_LEVEL_ERROR:
            return LOG_ERR;
        case LOG_LEVEL_WARNING:
            return LOG_WARNING;
        case LOG_LEVEL_INFO:
            return LOG_INFO;
        case LOG_LEVEL_DEBUG:
            return LOG_DEBUG;
        default:
            return LOG_INFO;
    }

    return LOG_INFO;
}

static const char* level_to_str(LogLevel level)
{
    switch (level)
    {
        case LOG_LEVEL_ERROR:
            return "Error";
        case LOG_LEVEL_WARNING:
            return "Warning";
        case LOG_LEVEL_INFO:
            return "Info";
        case LOG_LEVEL_DEBUG:
            return "Debug";
        default:
            return "??????";
    }

    return "??????";
}

void log_open(char *program_name, char *log_output, LogLevel level)
{
    if (!program_name || !log_output)
    {
        return;
    }

    gethostname(config_host_name, HOST_NAME_LENGTH);
    config_program_name = program_name;
    config_output_syslog = !strcmp(log_output, "syslog") ? 1 : 0;
    config_level = level;

    if (config_output_syslog)
    {
        openlog(program_name, LOG_CONS | LOG_PID, LOG_LOCAL0);
    }
}

void log_close(void)
{
    if (config_output_syslog)
    {
        closelog();
    }
}

void log_set_level(LogLevel level)
{
    config_level = level;
}

void log_printf(LogLevel level, char *format, ...)
{
    if (config_level < level)
    {
        return;
    }

    if (config_output_syslog)
    {
        char buf[LEVEL_STR_LENGTH + strlen(format)];
        snprintf(buf, sizeof(buf), "%s: %s", level_to_str(level), format);

        va_list va;
        va_start(va, format);

        vsyslog(level_to_syslog_level(level), buf, va);

        va_end(va);
    }
    else
    {
        struct timeval tv;
        struct timezone tz;
        gettimeofday(&tv, &tz);

        char* curr;
        curr = ctime(&tv.tv_sec);

        char *host_name;
        if (!strlen(config_host_name))
        {
            host_name = "??????";
        }
        else
        {
            host_name = config_host_name;
        }

        char *program_name;
        if (!config_program_name)
        {
            program_name = "??????";
        }
        else
        {
            program_name = config_program_name;
        }

        fprintf(stderr, "%15.15s.%03ld %s %s: %s:", curr + 4, tv.tv_usec / 1000,
                host_name, program_name, level_to_str(level));

        va_list va;
        va_start(va, format);

        vfprintf(stderr, format, va);

        va_end(va);

        fprintf(stderr, "\n");
    }
}
