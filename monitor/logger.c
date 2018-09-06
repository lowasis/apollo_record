#include <stdio.h>
#include <stdarg.h>
#include "logger.h"


int logger_init(char *name, LoggerContext *context)
{
    if (!name || !context)
    {
        return -1;
    }

    context->fp = fopen(name, "w");
    if (!context->fp)
    {
        fprintf(stderr, "Could not open log file\n");

        return -1;
    }

    context->level = LOGGER_LEVEL_DEFAULT;

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

    va_list va;
    va_start(va, format);

    ret = vfprintf(context->fp, format, va);
    if (ret == 0)
    {
        fprintf(stderr, "Could not write log\n");

        va_end(va);

        return -1;
    }

    va_end(va);

    return 0;
}
