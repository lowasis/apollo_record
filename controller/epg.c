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

    Py_Initialize();

    Py_SetProgramName("epg2xml");

    char str[8];
    snprintf(str, sizeof(str), "%d", channel);
    char *python_argv[] = {"epg2xml", "-c", str, "--episode", "n",
                           "--verbose", "n", "-o", context->name};
    int python_argc = sizeof(python_argv) / sizeof(python_argv[0]);
    PySys_SetArgv(python_argc, python_argv);

    ret = PyRun_SimpleString(___external_epg2xml_epg2xml_py);
    if (ret != 0)
    {
        fprintf(stderr, "Could not run python simple string\n");

        Py_Finalize();

        return NULL;
    }

    if (callback)
    {
        callback(context, channel, callback_arg);
    }

    Py_Finalize();

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

int epg_init(char *name, EpgContext *context)
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

    return 0;
}

void epg_uninit(EpgContext *context)
{
    if (!context)
    {
        return;
    }

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
