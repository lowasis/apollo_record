#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <python2.7/Python.h>
#include <libxml/xmlreader.h>
#include "epg2xml.h"
#include "epg.h"


#define XML_ENCODING        "UTF-8"


typedef struct RequestDataThreadArg {
    EpgContext *context;
    int channel;
    EpgRequestDataCallback callback;
    void *callback_arg;
} RequestDataThreadArg;

typedef struct ChannelTable {
    int channel;
    int epg2xml_id;
} ChannelTable;


static const ChannelTable channel_table_skb[] = {
    {2, 170}, {3, 265}, {4, 314}, {5, 164}, {6, 40}, {7, 111},
    {8, 312}, {9, 110}, {10, 238}, {11, 122}, {12, 85}, {13, 60},
    {14, 144}, {15, 97}, {16, 129}, {17, 207}, {18, 281}, {19, 208},
    {20, 149}, {21, 11}, {22, 260}, {23, 268}, {24, 218}, {25, 119},
    {26, 165}, {27, 121}, {28, 123}, {29, 84}, {30, 103}, {31, 258},
    {32, 128}, {33, 41}, {34, 148}, {35, 237}, {36, 231}, {37, 213},
    {38, 232}, {39, 313}, {40, 173}, {41, 143}, {42, 89}, {43, 48},
    {44, 62}, {45, 233}, {46, 230}, {47, 91}, {51, 290}, {52, 291},
    {53, 283}, {54, 151}, {55, 196}, {56, 171}, {57, 134}, {58, 38},
    {59, 201}, {61, 275}, {62, 318}, {70, 39}, {71, 319}, {72, 7},
    {73, 320}, {74, 445}, {79, 52}, {80, 104}, {81, 166}, {82, 99},
    {83, 55}, {84, 147}, {85, 216}, {86, 26}, {87, 293}, {88, 101},
    {89, 50}, {90, 80}, {91, 79}, {92, 240}, {93, 43}, {94, 177},
    {95, 61}, {96, 409}, {97, 282}, {98, 130}, {100, 442}, {101, 74},
    {102, 10}, {103, 35}, {104, 278}, {105, 280}, {106, 8}, {107, 87},
    {108, 286}, {109, 199}, {118, 414}, {119, 415}, {120, 183}, {121, 107},
    {122, 169}, {123, 126}, {124, 127}, {125, 178}, {126, 100}, {127, 185},
    {128, 186}, {129, 93}, {130, 20}, {131, 167}, {132, 98}, {133, 200},
    {134, 68}, {135, 192}, {136, 153}, {137, 184}, {138, 492}, {139, 446},
    {150, 297}, {151, 303}, {152, 135}, {153, 241}, {154, 264}, {155, 272},
    {156, 255}, {157, 220}, {158, 46}, {159, 47}, {160, 18}, {161, 411},
    {162, 21}, {163, 36}, {164, 225}, {170, 202}, {171, 235}, {172, 234},
    {173, 4}, {174, 5}, {175, 250}, {176, 141}, {177, 289}, {178, 266},
    {179, 2}, {182, 496}, {183, 497}, {188, 493}, {189, 112}, {190, 105},
    {191, 228}, {192, 96}, {193, 296}, {194, 57}, {195, 14}, {200, 95},
    {201, 457}, {202, 56}, {203, 58}, {204, 59}, {205, 63}, {210, 154},
    {211, 71}, {212, 285}, {213, 456}, {214, 108}, {215, 316}, {216, 75},
    {217, 86}, {218, 229}, {219, 150}, {220, 1}, {221, 321}, {222, 495},
    {230, 168}, {231, 124}, {232, 82}, {233, 92}, {234, 6}, {235, 42},
    {236, 94}, {237, 412}, {238, 273}, {239, 458}, {240, 245}, {241, 118},
    {242, 251}, {243, 78}, {244, 72}, {245, 156}, {246, 179}, {247, 133},
    {249, 302}, {251, 239}, {260, 138}, {261, 51}, {262, 219}, {263, 136},
    {264, 90}, {265, 15}, {266, 137}, {267, 160}, {268, 311}, {269, 294},
    {270, 263}, {271, 257}, {272, 198}, {273, 307}, {274, 125}, {275, 256},
    {276, 315}, {277, 31}, {278, 139}, {279, 205}, {280, 246}, {281, 284},
    {282, 222}, {290, 117}, {291, 223}, {292, 157}, {293, 247}, {300, 28},
    {301, 49}, {302, 32}, {303, 83}, {304, 24}, {305, 23}, {306, 19},
    {307, 301}, {308, 193}, {309, 215}, {320, 159}, {321, 244}, {322, 212},
    {323, 309}, {324, 299}, {325, 494},
    {0, 0}
};

static const ChannelTable channel_table_kt[] = {
    {2, 260}, {3, 123}, {4, 40}, {5, 164}, {6, 238}, {7, 111},
    {8, 85}, {9, 110}, {10, 312}, {11, 122}, {12, 144}, {13, 60},
    {14, 314}, {15, 97}, {16, 129}, {17, 207}, {18, 281}, {19, 208},
    {20, 119}, {21, 151}, {22, 265}, {23, 268}, {24, 218}, {25, 165},
    {26, 149}, {27, 121}, {28, 41}, {29, 283}, {30, 11}, {31, 173},
    {32, 196}, {33, 258}, {34, 148}, {35, 103}, {36, 313}, {37, 170},
    {38, 84}, {39, 99}, {40, 213}, {41, 104}, {42, 143}, {43, 166},
    {44, 237}, {45, 147}, {46, 231}, {47, 232}, {48, 55}, {49, 177},
    {50, 174}, {51, 183}, {52, 186}, {53, 93}, {54, 178}, {55, 200},
    {56, 98}, {57, 167}, {58, 169}, {59, 107}, {60, 126}, {61, 127},
    {62, 100}, {63, 192}, {64, 117}, {65, 223}, {66, 290}, {67, 291},
    {68, 48}, {69, 26}, {70, 230}, {71, 79}, {72, 409}, {73, 86},
    {74, 89}, {76, 216}, {77, 154}, {78, 456}, {79, 62}, {80, 172},
    {81, 150}, {82, 229}, {83, 108}, {84, 182}, {85, 293}, {87, 101},
    {88, 82}, {89, 34}, {90, 42}, {91, 6}, {92, 92}, {93, 221},
    {94, 455}, {95, 61}, {96, 168}, {97, 124}, {98, 282}, {99, 130},
    {100, 179}, {101, 39}, {102, 286}, {103, 134}, {104, 201}, {106, 171},
    {107, 74}, {108, 35}, {109, 7}, {110, 278}, {111, 8}, {112, 87},
    {113, 10}, {114, 199}, {115, 233}, {116, 20}, {117, 133}, {118, 78},
    {119, 72}, {120, 245}, {121, 118}, {122, 251}, {123, 153}, {124, 184},
    {125, 185}, {126, 43}, {127, 160}, {128, 495}, {129, 302}, {130, 235},
    {131, 53}, {132, 202}, {133, 4}, {134, 5}, {135, 2}, {136, 141},
    {137, 289}, {138, 266}, {139, 250}, {141, 228}, {142, 96}, {143, 497},
    {144, 105}, {145, 57}, {146, 14}, {148, 296}, {149, 115}, {151, 234},
    {152, 27}, {153, 496}, {154, 95}, {155, 457}, {156, 56}, {157, 58},
    {158, 59}, {159, 63}, {160, 157}, {161, 112}, {162, 493}, {163, 1},
    {164, 125}, {165, 176}, {166, 315}, {167, 175}, {168, 138}, {169, 90},
    {170, 137}, {171, 136}, {172, 15}, {174, 181}, {175, 219}, {176, 285},
    {177, 51}, {179, 3}, {180, 303}, {181, 135}, {182, 241}, {183, 272},
    {184, 255}, {185, 297}, {186, 264}, {188, 249}, {190, 220}, {191, 46},
    {192, 18}, {193, 67}, {194, 411}, {195, 76}, {196, 21}, {197, 45},
    {198, 205}, {199, 139}, {200, 263}, {201, 52}, {203, 287}, {204, 212},
    {205, 244}, {206, 159}, {207, 309}, {208, 299}, {209, 494}, {211, 414},
    {212, 415}, {213, 246}, {214, 198}, {217, 271}, {219, 247}, {220, 294},
    {221, 442}, {222, 454}, {223, 450}, {224, 451}, {225, 452}, {226, 444},
    {230, 116}, {231, 301}, {232, 19}, {233, 23}, {234, 83}, {235, 24},
    {236, 49}, {237, 32}, {238, 28}, {250, 284}, {251, 203}, {252, 307},
    {253, 91}, {254, 410}, {255, 257}, {257, 54}, {258, 9}, {259, 305},
    {260, 222}, {261, 193}, {262, 44}, {263, 273}, {264, 447}, {265, 256},
    {266, 262}, {267, 194}, {270, 292}, {271, 311}, {273, 416}, {276, 239},
    {277, 275}, {278, 253}, {279, 279}, {280, 31}, {281, 106}, {282, 254},
    {283, 226}, {284, 215}, {285, 225},
    {0, 0}
};

static const ChannelTable channel_table_lgu[] = {
    {2, 39}, {3, 104}, {4, 314}, {5, 164}, {6, 85}, {7, 111},
    {8, 40}, {9, 110}, {10, 312}, {11, 122}, {12, 238}, {13, 144},
    {14, 60}, {15, 97}, {16, 129}, {17, 207}, {18, 281}, {19, 208},
    {20, 265}, {21, 237}, {22, 121}, {23, 268}, {24, 218}, {25, 219},
    {26, 149}, {27, 165}, {28, 11}, {29, 123}, {30, 84}, {31, 103},
    {32, 41}, {33, 170}, {34, 313}, {35, 128}, {38, 151}, {39, 283},
    {40, 196}, {41, 171}, {42, 38}, {44, 74}, {45, 10}, {46, 134},
    {47, 201}, {48, 290}, {49, 291}, {50, 100}, {51, 185}, {52, 186},
    {53, 167}, {54, 98}, {55, 200}, {56, 183}, {57, 178}, {58, 169},
    {59, 107}, {60, 126}, {61, 127}, {62, 93}, {63, 20}, {64, 78},
    {65, 72}, {66, 179}, {67, 302}, {68, 62}, {69, 133}, {71, 147},
    {72, 216}, {73, 154}, {74, 260}, {75, 166}, {76, 258}, {77, 108},
    {79, 173}, {80, 286}, {81, 199}, {82, 148}, {83, 456}, {84, 229},
    {85, 48}, {86, 26}, {87, 278}, {88, 8}, {89, 52}, {90, 233},
    {91, 89}, {92, 495}, {93, 409}, {94, 99}, {95, 61}, {96, 153},
    {97, 245}, {98, 251}, {99, 124}, {100, 168}, {101, 82}, {102, 221},
    {103, 273}, {104, 55}, {105, 101}, {106, 92}, {107, 118}, {108, 293},
    {109, 184}, {110, 87}, {111, 297}, {112, 241}, {113, 264}, {115, 282},
    {116, 130}, {117, 46}, {118, 45}, {119, 411}, {120, 31}, {121, 303},
    {122, 135}, {123, 272}, {124, 255}, {125, 220}, {126, 18}, {130, 15},
    {131, 138}, {132, 90}, {133, 317}, {134, 137}, {135, 158}, {136, 86},
    {137, 150}, {138, 311}, {139, 6}, {140, 125}, {141, 263}, {142, 321},
    {143, 139}, {144, 294}, {145, 35}, {146, 42}, {148, 2}, {149, 266},
    {150, 235}, {151, 234}, {152, 202}, {153, 5}, {154, 141}, {155, 289},
    {156, 228}, {157, 296}, {158, 112}, {159, 96}, {160, 95}, {161, 457},
    {162, 56}, {163, 58}, {164, 59}, {165, 63}, {166, 250}, {167, 4},
    {168, 57}, {169, 105}, {170, 157}, {171, 117}, {172, 223}, {173, 247},
    {174, 222}, {175, 257}, {176, 198}, {177, 256}, {178, 1}, {180, 49},
    {181, 28}, {182, 24}, {183, 32}, {184, 301}, {185, 23}, {186, 19},
    {187, 193}, {188, 215}, {290, 159}, {291, 244}, {292, 212}, {293, 309},
    {294, 494}, {295, 299},
    {0, 0}
};

static const struct {
    EpgBroadcastServiceOperator oper;
    const ChannelTable *channel_table;
} operator_table[] = {
    {EPG_BROADCAST_SERVICE_OPERATOR_SKB, channel_table_skb},
    {EPG_BROADCAST_SERVICE_OPERATOR_KT, channel_table_kt},
    {EPG_BROADCAST_SERVICE_OPERATOR_LGU, channel_table_lgu},
    {0, NULL}
};

static pthread_mutex_t libpython_mutex;


static void *request_data_thread(void *arg)
{
    int ret;

    if (!arg)
    {
        return NULL;
    }

    EpgContext *context = ((RequestDataThreadArg *)arg)->context;
    int channel = ((RequestDataThreadArg *)arg)->channel;
    EpgRequestDataCallback callback = ((RequestDataThreadArg *)arg)->callback;
    void *callback_arg = ((RequestDataThreadArg *)arg)->callback_arg;

    free(arg);

    const ChannelTable *table = NULL;
    for (int i = 0; operator_table[i].channel_table; i++)
    {
        if (context->oper == operator_table[i].oper)
        {
            table = operator_table[i].channel_table;
            break;
        }
    }

    int epg2xml_id = 0;
    if (table)
    {
        for (int i = 0; table[i].epg2xml_id; i++)
        {
            if (channel == table[i].channel)
            {
                epg2xml_id = table[i].epg2xml_id;
                break;
            }
        }
    }

    ret = pthread_mutex_lock(&libpython_mutex);
    if (ret != 0)
    {
        fprintf(stderr, "Could not lock mutex\n");

        return NULL;
    }

    Py_Initialize();

    Py_SetProgramName("epg2xml");

    char str[8];
    snprintf(str, sizeof(str), "%d", epg2xml_id);
    char *python_argv[] = {"epg2xml", "-c", str, "--episode", "n",
                           "--verbose", "n", "-o", context->name};
    int python_argc = sizeof(python_argv) / sizeof(python_argv[0]);
    PySys_SetArgv(python_argc, python_argv);

    ret = PyRun_SimpleString(___external_epg2xml_epg2xml_py);
    if (ret != 0)
    {
        fprintf(stderr, "Could not run python simple string\n");

        Py_Finalize();

        ret = pthread_mutex_unlock(&libpython_mutex);
        if (ret != 0)
        {
            fprintf(stderr, "Could not unlock mutex\n");

            return NULL;
        }

        return NULL;
    }

    if (callback)
    {
        callback(context, channel, callback_arg);
    }

    Py_Finalize();

    ret = pthread_mutex_unlock(&libpython_mutex);
    if (ret != 0)
    {
        fprintf(stderr, "Could not unlock mutex\n");

        return NULL;
    }

    return NULL;
}

static int parse_epg_xml(char *buffer, int size, EpgData **data, int *count)
{
    int ret;

    if (!buffer || !data || !count)
    {
        return -1;
    }

    xmlTextReader *reader;
    reader = xmlReaderForMemory(buffer, size, "", XML_ENCODING, 0);
    if (!reader)
    {
        fprintf(stderr, "Could not get xml reader\n");

        xmlCleanupParser();

        return -1;
    }

    xmlReaderTypes type;
    const xmlChar *str;

    while (1)
    {
        type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_ELEMENT)
        {
            break;
        }

        ret = xmlTextReaderRead(reader);
        if (ret != 1)
        {
            fprintf(stderr, "Could not read xml\n");

            xmlFreeTextReader(reader);

            xmlCleanupParser();

            return -1;
        }
    }

    str = xmlTextReaderName(reader);
    if (!str)
    {
        fprintf(stderr, "Could not read xml element name\n");

        xmlFreeTextReader(reader);

        xmlCleanupParser();

        return -1;
    }

    if (xmlStrcmp(str, BAD_CAST("tv")))
    {
        xmlFree(BAD_CAST(str));

        xmlFreeTextReader(reader);

        xmlCleanupParser();

        return -1;
    }

    xmlFree(BAD_CAST(str));

    ret = xmlTextReaderRead(reader);
    if (ret != 1)
    {
        fprintf(stderr, "Could not read xml\n");

        xmlFreeTextReader(reader);

        xmlCleanupParser();

        return -1;
    }

    *data = NULL;
    *count = 0;

    while (1)
    {
        while (1)
        {
            while (1)
            {
                type = xmlTextReaderNodeType(reader);
                if (type == XML_READER_TYPE_ELEMENT)
                {
                    break;
                }
                else if (type == XML_READER_TYPE_END_ELEMENT)
                {
                    str = xmlTextReaderName(reader);
                    if (!str)
                    {
                        fprintf(stderr, "Could not read xml element name\n");

                        if (*data)
                        {
                            free(*data);
                            *data = NULL;
                        }
                        *count = 0;

                        xmlFreeTextReader(reader);

                        xmlCleanupParser();

                        return -1;
                    }

                    if (!xmlStrcmp(str, BAD_CAST("tv")))
                    {
                        xmlFree(BAD_CAST(str));

                        xmlFreeTextReader(reader);

                        xmlCleanupParser();

                        return 0;
                    }

                    xmlFree(BAD_CAST(str));
                }

                ret = xmlTextReaderRead(reader);
                if (ret != 1)
                {
                    fprintf(stderr, "Could not read xml\n");

                    if (*data)
                    {
                        free(*data);
                        *data = NULL;
                    }
                    *count = 0;

                    xmlFreeTextReader(reader);

                    xmlCleanupParser();

                    return -1;
                }
            }

            str = xmlTextReaderName(reader);
            if (!str)
            {
                fprintf(stderr, "Could not read xml element name\n");

                if (*data)
                {
                    free(*data);
                    *data = NULL;
                }
                *count = 0;

                xmlFreeTextReader(reader);

                xmlCleanupParser();

                return -1;
            }

            if (!xmlStrcmp(str, BAD_CAST("programme")))
            {
                xmlFree(BAD_CAST(str));

                break;
            }

            xmlFree(BAD_CAST(str));

            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                if (*data)
                {
                    free(*data);
                    *data = NULL;
                }
                *count = 0;

                xmlFreeTextReader(reader);

                xmlCleanupParser();

                return -1;
            }
        }

        *data = (EpgData *)realloc(*data, sizeof(EpgData) * (*count + 1));
        if (!*data)
        {
            fprintf(stderr, "Could not reallocate EPG data buffer\n");

            if (*data)
            {
                free(*data);
                *data = NULL;
            }
            *count = 0;

            xmlFreeTextReader(reader);

            xmlCleanupParser();

            return -1;
        }

        str = xmlTextReaderGetAttribute(reader, BAD_CAST("start"));
        if (str)
        {
            strncpy((*data)[*count].start, str, sizeof((*data)[*count].start));
        }
        else
        {
            fprintf(stderr, "Could not find xml attribute\n");

            if (*data)
            {
                free(*data);
                *data = NULL;
            }
            *count = 0;

            xmlFreeTextReader(reader);

            xmlCleanupParser();

            return -1;
        }

        xmlFree(BAD_CAST(str));

        str = xmlTextReaderGetAttribute(reader, BAD_CAST("stop"));
        if (str)
        {
            strncpy((*data)[*count].stop, str, sizeof((*data)[*count].stop));
        }
        else
        {
            fprintf(stderr, "Could not find xml attribute\n");

            if (*data)
            {
                free(*data);
                *data = NULL;
            }
            *count = 0;

            xmlFreeTextReader(reader);

            xmlCleanupParser();

            return -1;
        }

        xmlFree(BAD_CAST(str));

        ret = xmlTextReaderRead(reader);
        if (ret != 1)
        {
            fprintf(stderr, "Could not read xml\n");

            if (*data)
            {
                free(*data);
                *data = NULL;
            }
            *count = 0;

            xmlFreeTextReader(reader);

            xmlCleanupParser();

            return -1;
        }

        while (1)
        {
            while (1)
            {
                type = xmlTextReaderNodeType(reader);
                if (type == XML_READER_TYPE_ELEMENT)
                {
                    break;
                }

                ret = xmlTextReaderRead(reader);
                if (ret != 1)
                {
                    fprintf(stderr, "Could not read xml\n");

                    if (*data)
                    {
                        free(*data);
                        *data = NULL;
                    }
                    *count = 0;

                    xmlFreeTextReader(reader);

                    xmlCleanupParser();

                    return -1;
                }
            }

            str = xmlTextReaderName(reader);
            if (!str)
            {
                fprintf(stderr, "Could not read xml element name\n");

                if (*data)
                {
                    free(*data);
                    *data = NULL;
                }
                *count = 0;

                xmlFreeTextReader(reader);

                xmlCleanupParser();

                return -1;
            }

            if (!xmlStrcmp(str, BAD_CAST("title")))
            {
                xmlFree(BAD_CAST(str));

                break;
            }

            xmlFree(BAD_CAST(str));

            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                if (*data)
                {
                    free(*data);
                    *data = NULL;
                }
                *count = 0;

                xmlFreeTextReader(reader);

                xmlCleanupParser();

                return -1;
            }
        }

        ret = xmlTextReaderRead(reader);
        if (ret != 1)
        {
            fprintf(stderr, "Could not read xml\n");

            if (*data)
            {
                free(*data);
                *data = NULL;
            }
            *count = 0;

            xmlFreeTextReader(reader);

            xmlCleanupParser();

            return -1;
        }

        while (1)
        {
            type = xmlTextReaderNodeType(reader);
            if (type == XML_READER_TYPE_TEXT)
            {
                break;
            }

            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                if (*data)
                {
                    free(*data);
                    *data = NULL;
                }
                *count = 0;

                xmlFreeTextReader(reader);

                xmlCleanupParser();

                return -1;
            }
        }

        str = xmlTextReaderValue(reader);
        if (str)
        {
            strncpy((*data)[*count].title, str, sizeof((*data)[*count].title));
        }
        else
        {
            fprintf(stderr, "Could not find xml value\n");

            if (*data)
            {
                free(*data);
                *data = NULL;
            }
            *count = 0;

            xmlFreeTextReader(reader);

            xmlCleanupParser();

            return -1;
        }

        xmlFree(BAD_CAST(str));

        ret = xmlTextReaderRead(reader);
        if (ret != 1)
        {
            fprintf(stderr, "Could not read xml\n");

            if (*data)
            {
                free(*data);
                *data = NULL;
            }
            *count = 0;

            xmlFreeTextReader(reader);

            xmlCleanupParser();

            return -1;
        }

        (*count)++;
    }

    xmlFreeTextReader(reader);

    xmlCleanupParser();

    return 0;
}

int epg_init(char *name, EpgBroadcastServiceOperator oper, EpgContext *context)
{
    int ret;

    if (!name || !context)
    {
        return -1;
    }

    remove(name);

    context->name = (char *)malloc(strlen(name) + 1);
    if (!context->name)
    {
        fprintf(stderr, "Could not allocate EPG result file name buffer\n");

        return -1;
    }

    strncpy(context->name, name, strlen(name) + 1);

    if (oper < EPG_BROADCAST_SERVICE_OPERATOR_MIN ||
        EPG_BROADCAST_SERVICE_OPERATOR_MAX < oper)
    {
        fprintf(stderr, "Could not find broadcast service operator\n");

        free(context->name);

        return -1;
    }

    context->oper = oper;

    pthread_mutex_init(&libpython_mutex, NULL);

    return 0;
}

void epg_uninit(EpgContext *context)
{
    if (!context)
    {
        return;
    }

    pthread_mutex_destroy(&libpython_mutex);

    free(context->name);
}

int epg_request_data(EpgContext *context, int channel,
                     EpgRequestDataCallback callback, void *callback_arg)
{
    int ret;

    if (!context)
    {
        return -1;
    }

    RequestDataThreadArg *arg;
    arg = (RequestDataThreadArg *)malloc(sizeof(RequestDataThreadArg));
    if (!arg)
    {
        fprintf(stderr,
                "Could not allocate EPG request data thread argument buffer\n");

        return -1;
    }

    arg->context = context;
    arg->channel = channel;
    arg->callback = callback;
    arg->callback_arg = callback_arg;

    pthread_t thread;
    ret = pthread_create(&thread, NULL, request_data_thread, (void *)arg);
    if (ret < 0)
    {
        fprintf(stderr, "Could not create EPG request data thread\n");

        return -1;
    }

    return 0;
}

int epg_receive_data(EpgContext *context, EpgData **data, int *count)
{
    int ret;

    if (!context || !data || !count)
    {
        return -1;
    }

    FILE *fp;
    fp = fopen(context->name, "r");
    if (!fp)
    {
        fprintf(stderr, "Could not open EPG result file\n");

        return -1;
    }

    ret = fseek(fp, 0, SEEK_END);
    if (ret == -1)
    {
        fprintf(stderr, "Could not seek EPG result file to end\n");

        fclose(fp);

        return -1;
    }

    int size;
    size = ftell(fp);

    char *buffer;
    buffer = (char *)malloc(size);
    if (!buffer)
    {
        fprintf(stderr, "Could not allocate EPG receive buffer\n");

        fclose(fp);

        return -1;
    }

    ret = fseek(fp, 0, 0);
    if (ret == -1)
    {
        fprintf(stderr, "Could not seek EPG result file to start\n");

        free(buffer);

        fclose(fp);

        return -1;
    }

    ret = fread(buffer, 1, size, fp);
    if (ret != size)
    {
        fprintf(stderr,
                "Could not read EPG result file entirely (%d of %d bytes)\n",
                ret, size);

        free(buffer);

        fclose(fp);

        return -1;
    }

    fclose(fp);

    ret = parse_epg_xml(buffer, size, data, count);
    if (ret != 0)
    {
        fprintf(stderr, "Could not parse EPG xml\n");

        free(buffer);

        return -1;
    }

    free(buffer);

    return 0;
}
