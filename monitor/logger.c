#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include "log.h"
#include "logger.h"


int logger_init(char *name, LoggerLevel level, int use_timestamp,
                LoggerContext *context)
{
    if (!name || !context)
    {
        return -1;
    }

    context->fp = fopen(name, "w");
    if (!context->fp)
    {
        log_e("Could not open log file");

        return -1;
    }

    context->level = level;
    context->use_timestamp = use_timestamp;

    return 0;
}

void logger_uninit(LoggerContext *context)
{
    if (!context || !context->fp)
    {
        return;
    }

    fclose(context->fp);
}

int logger_set_level(LoggerContext *context, LoggerLevel level)
{
    if (!context)
    {
        return -1;
    }

    context->level = level;

    return 0;
}

int logger_set_timestamp(LoggerContext *context, int use)
{
    if (!context)
    {
        return -1;
    }

    context->use_timestamp = use;

    return 0;
}

int logger_printf(LoggerContext *context, LoggerLevel level, char *format, ...)
{
    int ret;

    if (!context || !context->fp)
    {
        return -1;
    } 

    if (context->level < level)
    {
        return 0;
    }

    if (context->use_timestamp)
    {
        struct timeval unix_time;
        ret = gettimeofday(&unix_time, NULL);
        if (ret == -1)
        {
            log_e("Could not get unix time");

            return -1;
        }

        struct tm *local_time;
        local_time = localtime(&unix_time.tv_sec);

        ret = fprintf(context->fp, "[%04d-%02d-%02d %02d:%02d:%02d.%03ld] ",
                      local_time->tm_year + 1900, local_time->tm_mon + 1,
                      local_time->tm_mday, local_time->tm_hour,
                      local_time->tm_min, local_time->tm_sec,
                      unix_time.tv_usec / 1000);
        if (ret == 0)
        {
            log_e("Could not write timestamp");

            return -1;
        }
    }

    va_list va;
    va_start(va, format);

    ret = vfprintf(context->fp, format, va);
    if (ret == 0)
    {
        log_e("Could not write log");

        va_end(va);

        return -1;
    }

    va_end(va);

    return 0;
}
