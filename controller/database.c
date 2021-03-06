#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "log.h"
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
            else if (strcmp(col[j], "AV_RECORD_NAME") == 0)
            {
                strncpy(data[*i].av_record_name, argv[j],
                        sizeof(data->av_record_name));
            }
            else if (strcmp(col[j], "LOUDNESS_LOG_NAME") == 0)
            {
                strncpy(data[*i].loudness_log_name, argv[j],
                        sizeof(data->loudness_log_name));
            }
            else if (strcmp(col[j], "PROGRAM_DATA_UPDATED") == 0)
            {
                data[*i].program_data_updated = strtol(argv[j], NULL, 10);
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
            else if (strcmp(col[j], "CHANNEL_NAME") == 0)
            {
                strncpy(data[*i].channel_name, argv[j],
                        sizeof(data->channel_name));
            }
            else if (strcmp(col[j], "PROGRAM_NAME") == 0)
            {
                strncpy(data[*i].program_name, argv[j],
                        sizeof(data->program_name));
            }
            else if (strcmp(col[j], "PROGRAM_START") == 0)
            {
                strncpy(data[*i].program_start, argv[j],
                        sizeof(data->program_start));
            }
            else if (strcmp(col[j], "PROGRAM_END") == 0)
            {
                strncpy(data[*i].program_end, argv[j],
                        sizeof(data->program_end));
            }
            else if (strcmp(col[j], "LOUDNESS") == 0)
            {
                data[*i].loudness = strtod(argv[j], NULL);
            }
            else if (strcmp(col[j], "LOUDNESS_OFFSET") == 0)
            {
                data[*i].loudness_offset = strtod(argv[j], NULL);
            }
            else if (strcmp(col[j], "TYPE") == 0)
            {
                data[*i].type = strtol(argv[j], NULL, 10);
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
            else if (strcmp(col[j], "END") == 0)
            {
                strncpy(data[*i].end, argv[j], sizeof(data->end));
            }
            else if (strcmp(col[j], "CHANNEL") == 0)
            {
                data[*i].channel = strtol(argv[j], NULL, 10);
            }
            else if (strcmp(col[j], "CHANNEL_NAME") == 0)
            {
                strncpy(data[*i].channel_name, argv[j],
                        sizeof(data->channel_name));
            }
            else if (strcmp(col[j], "RECORD_NAME") == 0)
            {
                strncpy(data[*i].record_name, argv[j],
                        sizeof(data->record_name));
            }
        }

        (*i)++;
    }

    return 0;
}

static int get_user_loudness_data_callback(void *arg, int argc, char **argv,
                                           char **col)
{
    if (!arg || !argv || !col)
    {
        return -1;
    }

    DatabaseUserLoudnessData *data = (DatabaseUserLoudnessData *)arg;
    memset(data, 0, sizeof(DatabaseUserLoudnessData));

    for (int i = 0; i < argc; i++)
    {
        if (!argv[i] || !col[i])
        {
            return -1;
        }

        if (strcmp(col[i], "NAME") == 0)
        {
            strncpy(data->name, argv[i], sizeof(data->name));
        }
        else if (strcmp(col[i], "RECORD_NAME") == 0)
        {
            strncpy(data->record_name, argv[i], sizeof(data->record_name));
        }
    }

    return 0;
}

static int get_user_loudness_section_data_callback(void *arg, int argc,
                                                   char **argv, char **col)
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
        DatabaseUserLoudnessSectionData *data =
                           (DatabaseUserLoudnessSectionData *)((void **)arg)[0];
        memset(&data[*i], 0, sizeof(DatabaseUserLoudnessSectionData));

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
            else if (strcmp(col[j], "LOUDNESS") == 0)
            {
                data[*i].loudness = strtod(argv[j], NULL);
            }
            else if (strcmp(col[j], "COMMENT") == 0)
            {
                strncpy(data[*i].comment, argv[j], sizeof(data->comment));
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
        log_e("Could not open sqlite3");

        return -1;
    }

    char query[512];
    DatabaseStatusData t;
    snprintf(query, sizeof(query),
             "CREATE TABLE IF NOT EXISTS STATUS("
             "ID                    INTEGER PRIMARY KEY AUTOINCREMENT, "
             "IDX                   INTEGER NOT NULL, "
             "CHANNEL               INTEGER NOT NULL, "
             "RECORDING             INTEGER NOT NULL, "
             "AV_RECORD_NAME        CHAR(%ld) NOT NULL, "
             "LOUDNESS_LOG_NAME     CHAR(%ld) NOT NULL, "
             "PROGRAM_DATA_UPDATED  INTEGER NOT NULL);",
             sizeof(t.av_record_name), sizeof(t.loudness_log_name));
    ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
    if(ret != SQLITE_OK)
    {
        log_e("Could not execute sqlite3 create status table");

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
        log_e("Could not execute sqlite3 create schedule table");

        return -1;
    }

    DatabasePlaybackListData p;
    snprintf(query, sizeof(query),
             "CREATE TABLE IF NOT EXISTS PLAYBACK_LIST("
             "ID                INTEGER PRIMARY KEY AUTOINCREMENT, "
             "NAME              CHAR(%ld) NOT NULL, "
             "START             CHAR(%ld) NOT NULL, "
             "END               CHAR(%ld) NOT NULL, "
             "CHANNEL           INTEGER NOT NULL, "
             "CHANNEL_NAME      CHAR(%ld) NOT NULL, "
             "PROGRAM_NAME      CHAR(%ld) NOT NULL, "
             "PROGRAM_START     CHAR(%ld) NOT NULL, "
             "PROGRAM_END       CHAR(%ld) NOT NULL, "
             "LOUDNESS          REAL NOT NULL, "
             "LOUDNESS_OFFSET   REAL NOT NULL, "
             "TYPE              INTEGER NOT NULL);",
             sizeof(p.name), sizeof(p.start), sizeof(p.end),
             sizeof(p.channel_name), sizeof(p.program_name),
             sizeof(p.program_start), sizeof(p.program_end));
    ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
    if(ret != SQLITE_OK)
    {
        log_e("Could not execute sqlite3 create playback list table");

        return -1;
    }

    DatabaseLogListData l;
    snprintf(query, sizeof(query),
             "CREATE TABLE IF NOT EXISTS LOG_LIST("
             "ID            INTEGER PRIMARY KEY AUTOINCREMENT, "
             "NAME          CHAR(%ld) NOT NULL, "
             "START         CHAR(%ld) NOT NULL, "
             "END           CHAR(%ld) NOT NULL, "
             "CHANNEL       INTEGER NOT NULL, "
             "CHANNEL_NAME  CHAR(%ld) NOT NULL, "
             "RECORD_NAME   CHAR(%ld) NOT NULL);",
             sizeof(l.name), sizeof(l.start), sizeof(l.end),
             sizeof(l.channel_name), sizeof(l.record_name));
    ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
    if(ret != SQLITE_OK)
    {
        log_e("Could not execute sqlite3 create log list table");

        return -1;
    }

    DatabaseUserLoudnessData u;
    snprintf(query, sizeof(query),
             "CREATE TABLE IF NOT EXISTS USER_LOUDNESS("
             "ID          INTEGER PRIMARY KEY AUTOINCREMENT, "
             "NAME        CHAR(%ld) NOT NULL, "
             "RECORD_NAME CHAR(%ld) NOT NULL);",
             sizeof(u.name), sizeof(u.record_name));
    ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
    if(ret != SQLITE_OK)
    {
        log_e("Could not execute sqlite3 create user loudness table");

        return -1;
    }

    DatabaseUserLoudnessSectionData s;
    snprintf(query, sizeof(query),
             "CREATE TABLE IF NOT EXISTS USER_LOUDNESS_SECTION("
             "ID          INTEGER PRIMARY KEY AUTOINCREMENT, "
             "NAME        CHAR(%ld) NOT NULL, "
             "START       CHAR(%ld) NOT NULL, "
             "END         CHAR(%ld) NOT NULL, "
             "LOUDNESS    REAL NOT NULL, "
             "COMMENT     CHAR(%ld) NOT NULL);",
             sizeof(s.name), sizeof(s.start), sizeof(s.end), sizeof(s.comment));
    ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
    if(ret != SQLITE_OK)
    {
        log_e("Could not execute sqlite3 create user loudness section table");

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
        log_e("Could not execute sqlite3 count status data");

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
                 "INSERT OR REPLACE INTO STATUS (ID, IDX, CHANNEL, RECORDING, "
                 "AV_RECORD_NAME, LOUDNESS_LOG_NAME, PROGRAM_DATA_UPDATED) "
                 "VALUES ((SELECT ID FROM STATUS WHERE IDX = %d), "
                 "%d, %d, %d, \"%s\", \"%s\", %d);",
                 data[i].index, data[i].index, data[i].channel,
                 data[i].recording, data[i].av_record_name,
                 data[i].loudness_log_name, data[i].program_data_updated);

        ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
        if(ret != SQLITE_OK)
        {
            log_e("Could not execute sqlite3 replace status data");

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
             "SELECT IDX, CHANNEL, RECORDING, AV_RECORD_NAME, "
             "LOUDNESS_LOG_NAME, PROGRAM_DATA_UPDATED FROM STATUS;");

    int i = 0;
    void *arg[] = {data, &count, &i};
    ret = sqlite3_exec(context->db, query, get_status_data_callback,
                       (void *)arg, NULL);
    if(ret != SQLITE_OK)
    {
        log_e("Could not execute sqlite3 select status data");

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
        log_e("Could not execute sqlite3 count schedule data");

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
        log_e("Could not execute sqlite3 delete schedule data");

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
            log_e("Could not execute sqlite3 insert schedule data");

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
        log_e("Could not execute sqlite3 select schedule data");

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
        log_e("Could not execute sqlite3 count playback list data");

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
                 "(ID, NAME, START, END, CHANNEL, CHANNEL_NAME, PROGRAM_NAME, "
                 "PROGRAM_START, PROGRAM_END, LOUDNESS, LOUDNESS_OFFSET, TYPE) "
                 "VALUES ((SELECT ID FROM PLAYBACK_LIST WHERE NAME = \"%s\"), "
                 "\"%s\", \"%s\", \"%s\", %d, \"%s\", \"%s\", \"%s\", \"%s\", "
                 "%f, %f, %d);",
                 data[i].name, data[i].name, data[i].start, data[i].end,
                 data[i].channel, data[i].channel_name, data[i].program_name,
                 data[i].program_start, data[i].program_end, data[i].loudness,
                 data[i].loudness_offset, data[i].type);

        ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
        if(ret != SQLITE_OK)
        {
            log_e("Could not execute sqlite3 insert playback list data");

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
             "SELECT NAME, START, END, CHANNEL, CHANNEL_NAME, PROGRAM_NAME, "
             "PROGRAM_START, PROGRAM_END, LOUDNESS, LOUDNESS_OFFSET, TYPE "
             "FROM PLAYBACK_LIST;");

    int i = 0;
    void *arg[] = {data, &count, &i};
    ret = sqlite3_exec(context->db, query, get_playback_list_data_callback,
                       (void *)arg, NULL);
    if(ret != SQLITE_OK)
    {
        log_e("Could not execute sqlite3 select playback list data");

        return -1;
    }

    return 0;
}

int database_get_playback_list_data_one(DatabaseContext *context, char *name,
                                        DatabasePlaybackListData *data)
{
    int ret;

    if (!context || !name || !data)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT NAME, START, END, CHANNEL, CHANNEL_NAME, PROGRAM_NAME, "
             "PROGRAM_START, PROGRAM_END, LOUDNESS, LOUDNESS_OFFSET, TYPE "
             "FROM PLAYBACK_LIST "
             "WHERE NAME = \"%s\";", name);

    int count = 1;
    int i = 0;
    void *arg[] = {data, &count, &i};
    ret = sqlite3_exec(context->db, query, get_playback_list_data_callback,
                       (void *)arg, NULL);
    if(ret != SQLITE_OK)
    {
        log_e("Could not execute sqlite3 select playback list data one");

        return -1;
    }

    if (i == 0)
    {
        log_e("Could not find sqlite3 playback list data one ");

        return -1;
    }

    return 0;
}

int database_update_playback_list_end_data(DatabaseContext *context, char *name,
                                           char *end)
{
    int ret;

    if (!context || !name || !end)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "UPDATE PLAYBACK_LIST SET END = \"%s\" WHERE NAME = \"%s\";",
             end, name);

    ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
    if(ret != SQLITE_OK)
    {
        log_e("Could not execute sqlite3 update playback list end data");

        return -1;
    }

    return 0;
}

int database_update_playback_list_program_data(DatabaseContext *context,
                                               char *name, char *program_name,
                                               char *program_start,
                                               char *program_end)
{
    int ret;

    if (!context || !name || !program_name || !program_start || !program_end)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "UPDATE PLAYBACK_LIST SET PROGRAM_NAME = \"%s\", "
             "PROGRAM_START = \"%s\", PROGRAM_END = \"%s\" "
             "WHERE NAME = \"%s\";",
             program_name, program_start, program_end, name);

    ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
    if(ret != SQLITE_OK)
    {
        log_e("Could not execute sqlite3 update playback list program data");

        return -1;
    }

    return 0;
}

int database_update_playback_list_loudness_data(DatabaseContext *context,
                                                char *name, double loudness)
{
    int ret;

    if (!context || !name)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "UPDATE PLAYBACK_LIST SET LOUDNESS = %f WHERE NAME = \"%s\";",
             loudness, name);

    ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
    if(ret != SQLITE_OK)
    {
        log_e("Could not execute sqlite3 update playback list loudness data");

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
        log_e("Could not execute sqlite3 count log list data");

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
                 "(ID, NAME, START, END, CHANNEL, CHANNEL_NAME, RECORD_NAME) "
                 "VALUES ((SELECT ID FROM LOG_LIST WHERE NAME = \"%s\"), "
                 "\"%s\", \"%s\", \"%s\", %d, \"%s\", \"%s\");",
                 data[i].name, data[i].name, data[i].start, data[i].end,
                 data[i].channel, data[i].channel_name, data[i].record_name);

        ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
        if(ret != SQLITE_OK)
        {
            log_e("Could not execute sqlite3 insert log list data");

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
             "SELECT NAME, START, END, CHANNEL, CHANNEL_NAME, RECORD_NAME "
             "FROM LOG_LIST;");

    int i = 0;
    void *arg[] = {data, &count, &i};
    ret = sqlite3_exec(context->db, query, get_log_list_data_callback,
                       (void *)arg, NULL);
    if(ret != SQLITE_OK)
    {
        log_e("Could not execute sqlite3 select log list data");

        return -1;
    }

    return 0;
}

int database_update_log_list_end_data(DatabaseContext *context, char *name,
                                      char *end)
{
    int ret;

    if (!context || !name || !end)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "UPDATE LOG_LIST SET END = \"%s\" WHERE NAME = \"%s\";",
             end, name);

    ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
    if(ret != SQLITE_OK)
    {
        log_e("Could not execute sqlite3 update log list end data");

        return -1;
    }

    return 0;
}

int database_count_user_loudness_data(DatabaseContext *context, char **name,
                                      int count, int *counted_count)
{
    int ret;

    if (!context || !name || !counted_count)
    {
        return -1;
    }

    *counted_count = 0;
    for (int i = 0; i < count; i++)
    {
        char query[512];
        snprintf(query, sizeof(query),
                 "SELECT COUNT(*) FROM USER_LOUDNESS WHERE NAME = \"%s\";",
                 name[i]);

        int c = 0;
        ret = sqlite3_exec(context->db, query, get_count_callback, (void *)&c,
                           NULL);
        if(ret != SQLITE_OK)
        {
            log_e("Could not execute sqlite3 count user loudness data");

            return -1;
        }

        *counted_count += c;
    }

    return 0;
}

int database_set_user_loudness_data(DatabaseContext *context,
                                    DatabaseUserLoudnessData *data, int count)
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
                 "INSERT OR REPLACE INTO USER_LOUDNESS "
                 "(ID, NAME, RECORD_NAME) VALUES "
                 "((SELECT ID FROM USER_LOUDNESS WHERE NAME = \"%s\"), "
                 "\"%s\", \"%s\");",
                 data[i].name, data[i].name, data[i].record_name);

        ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
        if(ret != SQLITE_OK)
        {
            log_e("Could not execute sqlite3 insert user loudness data");

            return -1;
        }
    }

    return 0;
}

int database_get_user_loudness_data(DatabaseContext *context, char **name,
                                    DatabaseUserLoudnessData *data, int count)
{
    int ret;

    if (!context || !name || !data)
    {
        return -1;
    }

    for (int i = 0; i < count; i++)
    {
        char query[512];
        snprintf(query, sizeof(query),
                 "SELECT NAME, RECORD_NAME "
                 "FROM USER_LOUDNESS WHERE NAME = \"%s\";",
                 name[i]);

        ret = sqlite3_exec(context->db, query, get_user_loudness_data_callback,
                           (void *)&data[i], NULL);
        if(ret != SQLITE_OK)
        {
            log_e("Could not execute sqlite3 select user loudness data");

            return -1;
        }
    }

    return 0;
}

int database_count_user_loudness_section_data(DatabaseContext *context,
                                              char *name, int *count)
{
    int ret;

    if (!context || !name || !count)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT COUNT(*) FROM USER_LOUDNESS_SECTION WHERE NAME = \"%s\";",
             name);

    ret = sqlite3_exec(context->db, query, get_count_callback, (void *)count,
                       NULL);
    if(ret != SQLITE_OK)
    {
        log_e("Could not execute sqlite3 count user loudness section data");

        return -1;
    }

    return 0;
}

int database_set_user_loudness_section_data(DatabaseContext *context,
                                          DatabaseUserLoudnessSectionData *data,
                                          int count)
{
    int ret;

    if (!context || !data)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "DELETE FROM USER_LOUDNESS_SECTION WHERE NAME = \"%s\";",
             data->name);

    ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
    if(ret != SQLITE_OK)
    {
        log_e("Could not execute sqlite3 delete user loudness section data");

        return -1;
    }

    for (int i = 0; i < count; i++)
    {
        snprintf(query, sizeof(query),
                 "INSERT OR REPLACE INTO USER_LOUDNESS_SECTION "
                 "(NAME, START, END, LOUDNESS, COMMENT) VALUES "
                 "(\"%s\", \"%s\", \"%s\", %f, \"%s\");",
                 data[i].name, data[i].start, data[i].end, data[i].loudness,
                 data[i].comment);

        ret = sqlite3_exec(context->db, query, NULL, NULL, NULL);
        if(ret != SQLITE_OK)
        {
            log_e("Could not execute sqlite3 insert "
                  "user loudness section data");

            return -1;
        }
    }

    return 0;
}

int database_get_user_loudness_section_data(DatabaseContext *context,
                                          char *name,
                                          DatabaseUserLoudnessSectionData *data,
                                          int count)
{
    int ret;

    if (!context || !name || !data || !count)
    {
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT NAME, START, END, LOUDNESS, COMMENT "
             "FROM USER_LOUDNESS_SECTION WHERE NAME = \"%s\";",
             name);

    int i = 0;
    void *arg[] = {data, &count, &i};
    ret = sqlite3_exec(context->db, query,
                       get_user_loudness_section_data_callback, (void *)arg,
                       NULL);
    if(ret != SQLITE_OK)
    {
        log_e("Could not execute sqlite3 select user loudness section data");

        return -1;
    }

    return 0;
}
