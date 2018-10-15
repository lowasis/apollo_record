#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "database.h"


static int get_count_callback(void *arg, int argc, char **argv, char **col)
{
    if (!arg || !argv || !col)
    {
        return -1;
    }

    int *data = (int *)arg;
    *data = 0;

    for (int i = 0; i < argc; i++)
    {
        if (!argv[i] || !col[i])
        {
            return -1;
        }

        if (strcmp(col[i], "COUNT(*)") == 0)
        {
            *data = strtol(argv[i], NULL, 10);
        }
    }

    return 0;
}

static int get_status_data_callback(void *arg, int argc, char **argv,
                                    char **col)
{
    if (!arg || !((void **)arg)[0] || !((void **)arg)[1] ||
        !((void **)arg)[2] || !argv || !col)
    {
        return -1;
    }

    int *count = (int *)((void **)arg)[1];
    int *i = (int *)((void **)arg)[2];

    if (*i < *count)
    {
        DatabaseStatusData *data = (DatabaseStatusData *)((void **)arg)[0];
        memset(&data[*i], 0, sizeof(DatabaseStatusData));

        for (int j = 0; j < argc; j++)
        {
            if (!argv[j] || !col[j])
            {
                return -1;
            }

            if (strcmp(col[j], "IDX") == 0)
            {
                data[*i].index = strtol(argv[j], NULL, 10);
            }
            else if (strcmp(col[j], "CHANNEL") == 0)
            {
                data[*i].channel = strtol(argv[j], NULL, 10);
            }
            else if (strcmp(col[j], "RECORDING") == 0)
            {
                data[*i].recording = strtol(argv[j], NULL, 10);
            }
        }

        (*i)++;
    }

    return 0;
}

static int get_schedule_data_callback(void *arg, int argc, char **argv,
                                      char **col)
{
    if (!arg || !((void **)arg)[0] || !((void **)arg)[1] ||
        !((void **)arg)[2] || !argv || !col)
    {
        return -1;
    }

    int *count = (int *)((void **)arg)[1];
    int *i = (int *)((void **)arg)[2];

    if (*i < *count)
    {
        DatabaseScheduleData *data = (DatabaseScheduleData *)((void **)arg)[0];
        memset(&data[*i], 0, sizeof(DatabaseScheduleData));

        for (int j = 0; j < argc; j++)
        {
            if (!argv[j] || !col[j])
            {
                return -1;
            }

            if (strcmp(col[j], "IDX") == 0)
            {
                data[*i].index = strtol(argv[j], NULL, 10);
            }
            else if (strcmp(col[j], "START") == 0)
            {
                data[*i].start = strtol(argv[j], NULL, 10);
            }
            else if (strcmp(col[j], "END") == 0)
            {
                data[*i].end = strtol(argv[j], NULL, 10);
            }
            else if (strcmp(col[j], "CHANNEL") == 0)
            {
                data[*i].channel = strtol(argv[j], NULL, 10);
            }
        }

        (*i)++;
    }

    return 0;
}

static int get_playback_list_data_callback(void *arg, int argc, char **argv,
                                           char **col)
{
    if (!arg || !((void **)arg)[0] || !((void **)arg)[1] ||
        !((void **)arg)[2] || !argv || !col)
    {
        return -1;
    }

    int *count = (int *)((void **)arg)[1];
    int *i = (int *)((void **)arg)[2];

    if (*i < *count)
    {
        DatabasePlaybackListData *data =
                                  (DatabasePlaybackListData *)((void **)arg)[0];
        memset(&data[*i], 0, sizeof(DatabasePlaybackListData));

        for (int j = 0; j < argc; j++)
        {
            if (!argv[j] || !col[j])
            {
                return -1;
            }

            if (strcmp(col[j], "NAME") == 0)
            {
                strncpy(data[*i].name, argv[j], sizeof(data->name));
            }
            else if (strcmp(col[j], "START") == 0)
            {
                strncpy(data[*i].start, argv[j], sizeof(data->start));
            }
            else if (strcmp(col[j], "END") == 0)
            {
                strncpy(data[*i].end, argv[j], sizeof(data->end));
            }
            else if (strcmp(col[j], "CHANNEL") == 0)
            {
                data[*i].channel = strtol(argv[j], NULL, 10);
            }
        }

        (*i)++;
    }

    return 0;
}

static int get_log_list_data_callback(void *arg, int argc, char **argv,
                                      char **col)
{
    if (!arg || !((void **)arg)[0] || !((void **)arg)[1] ||
        !((void **)arg)[2] || !argv || !col)
    {
        return -1;
    }

    int *count = (int *)((void **)arg)[1];
    int *i = (int *)((void **)arg)[2];

    if (*i < *count)
    {
        DatabaseLogListData *data = (DatabaseLogListData *)((void **)arg)[0];
        memset(&data[*i], 0, sizeof(DatabaseLogListData));

        for (int j = 0; j < argc; j++)
        {
            if (!argv[j] || !col[j])
            {
                return -1;
            }

            if (strcmp(col[j], "NAME") == 0)
            {
                strncpy(data[*i].name, argv[j], sizeof(data->name));
            }
            else if (strcmp(col[j], "START") == 0)
            {
                strncpy(data[*i].start, argv[j], sizeof(data->start));
            }
            else if (strcmp(col[j], "CHANNEL") == 0)
            {
                data[*i].channel = strtol(argv[j], NULL, 10);
            }
        }

        (*i)++;
    }

    return 0;
}

int database_init(char *name, DatabaseContext *context)
{
    int ret;

    if (!name || !context)
    {
        return -1;
    }

    ret = sqlite3_open(name, &context->db);
    if (ret != SQLITE_OK)
    {
        fprintf(stderr, "Could not open sqlite3\n");

        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "CREATE TABLE IF NOT EXISTS STATUS("
             "ID          INTEGER PRIMARY KEY AUTOINCREMENT, "
             "IDX         INTEGER NOT NULL, "
             "CHANNEL     INTEGER NOT NULL, "
             "RECORDING   INTEGER NOT NULL);");
    ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr, "Could not execute sqlite3 create status table\n");

        return -1;
    }

    snprintf(query, sizeof(query),
             "CREATE TABLE IF NOT EXISTS SCHEDULE("
             "ID          INTEGER PRIMARY KEY AUTOINCREMENT, "
             "IDX         INTEGER NOT NULL, "
             "START       INTEGER NOT NULL, "
             "END         INTEGER NOT NULL, "
             "CHANNEL     INTEGER NOT NULL);");
    ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr, "Could not execute sqlite3 create schedule table\n");

        return -1;
    }

    DatabasePlaybackListData p;
    snprintf(query, sizeof(query),
             "CREATE TABLE IF NOT EXISTS PLAYBACK_LIST("
             "ID          INTEGER PRIMARY KEY AUTOINCREMENT, "
             "NAME        CHAR(%ld) NOT NULL, "
             "START       CHAR(%ld) NOT NULL, "
             "END         CHAR(%ld) NOT NULL, "
             "CHANNEL     INTEGER NOT NULL);",
             sizeof(p.name), sizeof(p.start), sizeof(p.end));
    ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr,
                "Could not execute sqlite3 create playback list table\n");

        return -1;
    }

    DatabaseLogListData l;
    snprintf(query, sizeof(query),
             "CREATE TABLE IF NOT EXISTS LOG_LIST("
             "ID          INTEGER PRIMARY KEY AUTOINCREMENT, "
             "NAME        CHAR(%ld) NOT NULL, "
             "START       CHAR(%ld) NOT NULL, "
             "CHANNEL     INTEGER NOT NULL);",
             sizeof(p.name), sizeof(p.start));
    ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr,
                "Could not execute sqlite3 create log list table\n");

        return -1;
    }

    return 0;
}

void database_uninit(DatabaseContext *context)
{
    if (!context)
    {
        return;
    }

    sqlite3_close(context->db);
}

int database_count_status_data(DatabaseContext *context, int *count)
{
    int ret;

    if (!context || !count)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query), "SELECT COUNT(*) FROM STATUS;");

    ret = sqlite3_exec(context->db, query, get_count_callback, (void *)count,
                       NULL);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr, "Could not execute sqlite3 count status data\n");

        return -1;
    }

    return 0;
}

int database_set_status_data(DatabaseContext *context, DatabaseStatusData *data,
                             int count)
{
    int ret;

    if (!context || !data)
    {
        return -1;
    }

    for (int i = 0; i < count; i++)
    {
        char query[512];
        snprintf(query, sizeof(query),
                 "INSERT OR REPLACE INTO STATUS (ID, IDX, CHANNEL, RECORDING) "
                 "VALUES ((SELECT ID FROM STATUS WHERE IDX = %d), %d, %d, %d);",
                 data[i].index, data[i].index, data[i].channel,
                 data[i].recording);

        ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
        if(ret != SQLITE_OK)
        {
            fprintf(stderr, "Could not execute sqlite3 replace status data\n");

            return -1;
        }
    }

    return 0;
}

int database_get_status_data(DatabaseContext *context, DatabaseStatusData *data,
                             int count)
{
    int ret;

    if (!context || !data)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT IDX, CHANNEL, RECORDING FROM STATUS;");

    int i = 0;
    void *arg[] = {data, &count, &i};
    ret = sqlite3_exec(context->db, query, get_status_data_callback,
                       (void *)arg, NULL);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr, "Could not execute sqlite3 select status data\n");

        return -1;
    }

    return 0;
}

int database_count_schedule_data(DatabaseContext *context, int *count)
{
    int ret;

    if (!context || !count)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query), "SELECT COUNT(*) FROM SCHEDULE;");

    ret = sqlite3_exec(context->db, query, get_count_callback, (void *)count,
                       NULL);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr, "Could not execute sqlite3 count schedule data\n");

        return -1;
    }

    return 0;
}

int database_set_schedule_data(DatabaseContext *context,
                               DatabaseScheduleData *data, int count)
{
    int ret;

    if (!context || !data)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query), "DELETE FROM SCHEDULE;");

    ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr, "Could not execute sqlite3 delete schedule data\n");

        return -1;
    }

    for (int i = 0; i < count; i++)
    {
        snprintf(query, sizeof(query),
                 "INSERT OR REPLACE INTO SCHEDULE (IDX, START, END, CHANNEL) "
                 "VALUES (%d, %ld, %ld, %d);",
                 data[i].index, data[i].start, data[i].end, data[i].channel);

        ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
        if(ret != SQLITE_OK)
        {
            fprintf(stderr, "Could not execute sqlite3 insert schedule data\n");

            return -1;
        }
    }

    return 0;
}

int database_get_schedule_data(DatabaseContext *context,
                               DatabaseScheduleData *data, int count)
{
    int ret;

    if (!context || !data)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT IDX, START, END, CHANNEL FROM SCHEDULE;");

    int i = 0;
    void *arg[] = {data, &count, &i};
    ret = sqlite3_exec(context->db, query, get_schedule_data_callback,
                       (void *)arg, NULL);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr, "Could not execute sqlite3 select schedule data\n");

        return -1;
    }

    return 0;
}

int database_count_playback_list_data(DatabaseContext *context, int *count)
{
    int ret;

    if (!context || !count)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query), "SELECT COUNT(*) FROM PLAYBACK_LIST;");

    ret = sqlite3_exec(context->db, query, get_count_callback, (void *)count,
                       NULL);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr, "Could not execute sqlite3 count playback list data\n");

        return -1;
    }

    return 0;
}

int database_set_playback_list_data(DatabaseContext *context,
                                    DatabasePlaybackListData *data, int count)
{
    int ret;

    if (!context || !data)
    {
        return -1;
    }

    for (int i = 0; i < count; i++)
    {
        char query[512];
        snprintf(query, sizeof(query),
                 "INSERT OR REPLACE INTO PLAYBACK_LIST "
                 "(ID, NAME, START, END, CHANNEL) VALUES "
                 "((SELECT ID FROM PLAYBACK_LIST WHERE NAME = \"%s\"), "
                 "\"%s\", \"%s\", \"%s\", %d);",
                 data[i].name, data[i].name, data[i].start, data[i].end,
                 data[i].channel);

        ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
        if(ret != SQLITE_OK)
        {
            fprintf(stderr,
                    "Could not execute sqlite3 insert playback list data\n");

            return -1;
        }
    }

    return 0;
}

int database_get_playback_list_data(DatabaseContext *context,
                                    DatabasePlaybackListData *data, int count)
{
    int ret;

    if (!context || !data)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT NAME, START, END, CHANNEL FROM PLAYBACK_LIST;");

    int i = 0;
    void *arg[] = {data, &count, &i};
    ret = sqlite3_exec(context->db, query, get_playback_list_data_callback,
                       (void *)arg, NULL);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr,
                "Could not execute sqlite3 select playback list data\n");

        return -1;
    }

    return 0;
}

int database_count_log_list_data(DatabaseContext *context, int *count)
{
    int ret;

    if (!context || !count)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query), "SELECT COUNT(*) FROM LOG_LIST;");

    ret = sqlite3_exec(context->db, query, get_count_callback, (void *)count,
                       NULL);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr, "Could not execute sqlite3 count log list data\n");

        return -1;
    }

    return 0;
}

int database_set_log_list_data(DatabaseContext *context,
                               DatabaseLogListData *data, int count)
{
    int ret;

    if (!context || !data)
    {
        return -1;
    }

    for (int i = 0; i < count; i++)
    {
        char query[512];
        snprintf(query, sizeof(query),
                 "INSERT OR REPLACE INTO LOG_LIST "
                 "(ID, NAME, START, CHANNEL) VALUES "
                 "((SELECT ID FROM LOG_LIST WHERE NAME = \"%s\"), "
                 "\"%s\", \"%s\", %d);",
                 data[i].name, data[i].name, data[i].start, data[i].channel);

        ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
        if(ret != SQLITE_OK)
        {
            fprintf(stderr,
                    "Could not execute sqlite3 insert log list data\n");

            return -1;
        }
    }

    return 0;
}

int database_get_log_list_data(DatabaseContext *context,
                               DatabaseLogListData *data, int count)
{
    int ret;

    if (!context || !data)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT NAME, START, CHANNEL FROM LOG_LIST;");

    int i = 0;
    void *arg[] = {data, &count, &i};
    ret = sqlite3_exec(context->db, query, get_log_list_data_callback,
                       (void *)arg, NULL);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr,
                "Could not execute sqlite3 select log list data\n");

        return -1;
    }

    return 0;
}