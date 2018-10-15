#ifndef DATABASE_H
#define DATABASE_H

#include <time.h>
#include <sqlite3.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DatabaseStatusData {
    int index;
    int channel;
    int recording;
} DatabaseStatusData;

typedef struct DatabaseScheduleData {
    int index;
    time_t start;
    time_t end;
    int channel;
} DatabaseScheduleData;

typedef struct DatabasePlaybackListData {
    char name[128];
    char start[24];
    char end[24];
    int channel;
} DatabasePlaybackListData;

typedef struct DatabaseLogListData {
    char name[128];
    char start[24];
    int channel;
} DatabaseLogListData;

typedef struct DatabaseUserLoudnessData {
    char name[128];
    char record_name[128];
} DatabaseUserLoudnessData;

typedef struct DatabaseUserLoudnessSectionData {
    char name[128];
    char start[16];
    char end[16];
    double loudness;
    char comment[128];
} DatabaseUserLoudnessSectionData;

typedef struct DatabaseContext {
    sqlite3 *db;
} DatabaseContext;

int database_init(char *name, DatabaseContext *context);
void database_uninit(DatabaseContext *context);
int database_count_status_data(DatabaseContext *context, int *count);
int database_set_status_data(DatabaseContext *context, DatabaseStatusData *data,
                             int count);
int database_get_status_data(DatabaseContext *context, DatabaseStatusData *data,
                             int count);
int database_count_schedule_data(DatabaseContext *context, int *count);
int database_set_schedule_data(DatabaseContext *context,
                               DatabaseScheduleData *data, int count);
int database_get_schedule_data(DatabaseContext *context,
                               DatabaseScheduleData *data, int count);
int database_count_playback_list_data(DatabaseContext *context, int *count);
int database_set_playback_list_data(DatabaseContext *context,
                                    DatabasePlaybackListData *data, int count);
int database_get_playback_list_data(DatabaseContext *context,
                                    DatabasePlaybackListData *data, int count);
int database_count_log_list_data(DatabaseContext *context, int *count);
int database_set_log_list_data(DatabaseContext *context,
                               DatabaseLogListData *data, int count);
int database_get_log_list_data(DatabaseContext *context,
                               DatabaseLogListData *data, int count);
int database_count_user_loudness_data(DatabaseContext *context, char **name,
                                      int count, int *counted_count);
int database_set_user_loudness_data(DatabaseContext *context,
                                    DatabaseUserLoudnessData *data, int count);
int database_get_user_loudness_data(DatabaseContext *context, char **name,
                                    DatabaseUserLoudnessData *data, int count);
int database_count_user_loudness_section_data(DatabaseContext *context,
                                              char *name, int *count);
int database_set_user_loudness_section_data(DatabaseContext *context,
                                          DatabaseUserLoudnessSectionData *data,
                                          int count);
int database_get_user_loudness_section_data(DatabaseContext *context,
                                          char *name,
                                          DatabaseUserLoudnessSectionData *data,
                                          int count);

#ifdef __cplusplus
}
#endif

#endif
