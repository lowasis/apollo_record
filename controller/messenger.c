#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>
#include "messenger.h"


#define XML_ENCODING        "UTF-8"
#define XML_HEADER          "<?xml"


static int generate_ack_xml(MessengerMessage *message, xmlTextWriter *writer)
{
    int ret;

    if (!message || !writer)
    {
        return -1;
    }

    ret = xmlTextWriterStartDocument(writer, NULL, XML_ENCODING, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not start xml document\n");

        return -1;
    }

    ret = xmlTextWriterStartElement(writer, BAD_CAST("ack"));
    if (ret < 0)
    {
        fprintf(stderr, "Could not start xml element\n");

        return -1;
    }

    ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("ip"), "%s",
                                            message->ip);
    if (ret < 0)
    {
        fprintf(stderr, "Could not write xml attribute\n");

        return -1;
    }

    ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("number"), "%d",
                                            message->number);
    if (ret < 0)
    {
        fprintf(stderr, "Could not write xml attribute\n");

        return -1;
    }

    ret = xmlTextWriterEndElement(writer);
    if (ret < 0)
    {
        fprintf(stderr, "Could not end xml element\n");

        return -1;
    }

    ret = xmlTextWriterEndDocument(writer);
    if (ret < 0)
    {
        fprintf(stderr, "Could not end xml document\n");

        return -1;
    }

    return 0;
}

static int generate_loudness_xml(MessengerMessage *message,
                                 xmlTextWriter *writer)
{
    int ret;

    if (!message || !writer)
    {
        return -1;
    }

    ret = xmlTextWriterStartDocument(writer, NULL, XML_ENCODING, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not start xml document\n");

        return -1;
    }

    ret = xmlTextWriterStartElement(writer, BAD_CAST("loudness"));
    if (ret < 0)
    {
        fprintf(stderr, "Could not start xml element\n");

        return -1;
    }

    ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("ip"), "%s",
                                            message->ip);
    if (ret < 0)
    {
        fprintf(stderr, "Could not write xml attribute\n");

        return -1;
    }

    MessengerLoudnessData *data;
    data = (MessengerLoudnessData *)message->data;
    for (int i = 0; i < message->count; i++)
    {
        ret = xmlTextWriterStartElement(writer, BAD_CAST("card"));
        if (ret < 0)
        {
            fprintf(stderr, "Could not start xml element\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("index"), "%d",
                                                data[i].index);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("reference"),
                                                "%2.1f", data[i].reference);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("momentary"),
                                                "%2.1f", data[i].momentary);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("short-term"),
                                                "%2.1f", data[i].shortterm);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("integrated"),
                                               "%2.1f", data[i].integrated);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterEndElement(writer);
        if (ret < 0)
        {
            fprintf(stderr, "Could not end xml element\n");

            return -1;
        }
    }

    ret = xmlTextWriterEndElement(writer);
    if (ret < 0)
    {
        fprintf(stderr, "Could not end xml element\n");

        return -1;
    }

    ret = xmlTextWriterEndDocument(writer);
    if (ret < 0)
    {
        fprintf(stderr, "Could not end xml document\n");

        return -1;
    }

    return 0;
}

static int generate_status_xml(MessengerMessage *message, xmlTextWriter *writer)
{
    int ret;

    if (!message || !writer)
    {
        return -1;
    }

    ret = xmlTextWriterStartDocument(writer, NULL, XML_ENCODING, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not start xml document\n");

        return -1;
    }

    ret = xmlTextWriterStartElement(writer, BAD_CAST("status"));
    if (ret < 0)
    {
        fprintf(stderr, "Could not start xml element\n");

        return -1;
    }

    ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("ip"), "%s",
                                            message->ip);
    if (ret < 0)
    {
        fprintf(stderr, "Could not write xml attribute\n");

        return -1;
    }

    MessengerStatusData *data;
    data = (MessengerStatusData *)message->data;
    for (int i = 0; i < message->count; i++)
    {
        ret = xmlTextWriterStartElement(writer, BAD_CAST("card"));
        if (ret < 0)
        {
            fprintf(stderr, "Could not start xml element\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("index"), "%d",
                                                data[i].index);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("channel"),
                                                "%d", data[i].channel);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("recording"),
                                                "%s", data[i].recording ?
                                                "true" : "false");
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterEndElement(writer);
        if (ret < 0)
        {
            fprintf(stderr, "Could not end xml element\n");

            return -1;
        }
    }

    ret = xmlTextWriterEndElement(writer);
    if (ret < 0)
    {
        fprintf(stderr, "Could not end xml element\n");

        return -1;
    }

    ret = xmlTextWriterEndDocument(writer);
    if (ret < 0)
    {
        fprintf(stderr, "Could not end xml document\n");

        return -1;
    }

    return 0;
}

static int generate_schedule_xml(MessengerMessage *message,
                                 xmlTextWriter *writer)
{
    int ret;

    if (!message || !writer)
    {
        return -1;
    }

    ret = xmlTextWriterStartDocument(writer, NULL, XML_ENCODING, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not start xml document\n");

        return -1;
    }

    ret = xmlTextWriterStartElement(writer, BAD_CAST("schedule"));
    if (ret < 0)
    {
        fprintf(stderr, "Could not start xml element\n");

        return -1;
    }

    ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("ip"), "%s",
                                            message->ip);
    if (ret < 0)
    {
        fprintf(stderr, "Could not write xml attribute\n");

        return -1;
    }

    ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("number"), "%d",
                                            message->number);
    if (ret < 0)
    {
        fprintf(stderr, "Could not write xml attribute\n");

        return -1;
    }

    MessengerScheduleData *data;
    data = (MessengerScheduleData *)message->data;
    for (int i = 0; i < message->count; i++)
    {
        ret = xmlTextWriterStartElement(writer, BAD_CAST("card"));
        if (ret < 0)
        {
            fprintf(stderr, "Could not start xml element\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("index"), "%d",
                                                data[i].index);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("start"),
                                                "%s", data[i].start);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("end"),
                                                "%s", data[i].end);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("channel"),
                                                "%d", data[i].channel);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterEndElement(writer);
        if (ret < 0)
        {
            fprintf(stderr, "Could not end xml element\n");

            return -1;
        }
    }

    ret = xmlTextWriterEndElement(writer);
    if (ret < 0)
    {
        fprintf(stderr, "Could not end xml element\n");

        return -1;
    }

    ret = xmlTextWriterEndDocument(writer);
    if (ret < 0)
    {
        fprintf(stderr, "Could not end xml document\n");

        return -1;
    }

    return 0;
}

static int generate_playback_list_xml(MessengerMessage *message,
                                      xmlTextWriter *writer)
{
    int ret;

    if (!message || !writer)
    {
        return -1;
    }

    ret = xmlTextWriterStartDocument(writer, NULL, XML_ENCODING, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not start xml document\n");

        return -1;
    }

    ret = xmlTextWriterStartElement(writer, BAD_CAST("playback_list"));
    if (ret < 0)
    {
        fprintf(stderr, "Could not start xml element\n");

        return -1;
    }

    ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("ip"), "%s",
                                            message->ip);
    if (ret < 0)
    {
        fprintf(stderr, "Could not write xml attribute\n");

        return -1;
    }

    ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("number"), "%d",
                                            message->number);
    if (ret < 0)
    {
        fprintf(stderr, "Could not write xml attribute\n");

        return -1;
    }

    MessengerPlaybackListData *data;
    data = (MessengerPlaybackListData *)message->data;
    for (int i = 0; i < message->count; i++)
    {
        ret = xmlTextWriterStartElement(writer, BAD_CAST("file"));
        if (ret < 0)
        {
            fprintf(stderr, "Could not start xml element\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("name"), "%s",
                                                data[i].name);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("start"),
                                                "%s", data[i].start);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("end"),
                                                "%s", data[i].end);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("channel"),
                                                "%d", data[i].channel);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer,
                                                BAD_CAST("channel_name"), "%s",
                                                data[i].channel_name);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer,
                                                BAD_CAST("program_name"), "%s",
                                                data[i].program_name);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer,
                                                BAD_CAST("program_start"), "%s",
                                                data[i].program_start);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("program_end"),
                                                "%s", data[i].program_end);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("loudness"),
                                                "%2.1f", data[i].loudness);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("type"),
                                                "%s", data[i].type ?
                                                "user" : "record");
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterEndElement(writer);
        if (ret < 0)
        {
            fprintf(stderr, "Could not end xml element\n");

            return -1;
        }
    }

    ret = xmlTextWriterEndElement(writer);
    if (ret < 0)
    {
        fprintf(stderr, "Could not end xml element\n");

        return -1;
    }

    ret = xmlTextWriterEndDocument(writer);
    if (ret < 0)
    {
        fprintf(stderr, "Could not end xml document\n");

        return -1;
    }

    return 0;
}

static int generate_log_list_xml(MessengerMessage *message,
                                 xmlTextWriter *writer)
{
    int ret;

    if (!message || !writer)
    {
        return -1;
    }

    ret = xmlTextWriterStartDocument(writer, NULL, XML_ENCODING, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not start xml document\n");

        return -1;
    }

    ret = xmlTextWriterStartElement(writer, BAD_CAST("log_list"));
    if (ret < 0)
    {
        fprintf(stderr, "Could not start xml element\n");

        return -1;
    }

    ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("ip"), "%s",
                                            message->ip);
    if (ret < 0)
    {
        fprintf(stderr, "Could not write xml attribute\n");

        return -1;
    }

    ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("number"), "%d",
                                            message->number);
    if (ret < 0)
    {
        fprintf(stderr, "Could not write xml attribute\n");

        return -1;
    }

    MessengerLogListData *data;
    data = (MessengerLogListData *)message->data;
    for (int i = 0; i < message->count; i++)
    {
        ret = xmlTextWriterStartElement(writer, BAD_CAST("file"));
        if (ret < 0)
        {
            fprintf(stderr, "Could not start xml element\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("name"), "%s",
                                                data[i].name);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("start"),
                                                "%s", data[i].start);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("end"),
                                                "%s", data[i].end);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("channel"),
                                                "%d", data[i].channel);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer,
                                                BAD_CAST("channel_name"), "%s",
                                                data[i].channel_name);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterEndElement(writer);
        if (ret < 0)
        {
            fprintf(stderr, "Could not end xml element\n");

            return -1;
        }
    }

    ret = xmlTextWriterEndElement(writer);
    if (ret < 0)
    {
        fprintf(stderr, "Could not end xml element\n");

        return -1;
    }

    ret = xmlTextWriterEndDocument(writer);
    if (ret < 0)
    {
        fprintf(stderr, "Could not end xml document\n");

        return -1;
    }

    return 0;
}

static int generate_user_loudness_xml(MessengerMessage *message,
                                      xmlTextWriter *writer)
{
    int ret;

    if (!message || !writer)
    {
        return -1;
    }

    ret = xmlTextWriterStartDocument(writer, NULL, XML_ENCODING, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not start xml document\n");

        return -1;
    }

    ret = xmlTextWriterStartElement(writer, BAD_CAST("user_loudness"));
    if (ret < 0)
    {
        fprintf(stderr, "Could not start xml element\n");

        return -1;
    }

    ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("ip"), "%s",
                                            message->ip);
    if (ret < 0)
    {
        fprintf(stderr, "Could not write xml attribute\n");

        return -1;
    }

    ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("number"), "%d",
                                            message->number);
    if (ret < 0)
    {
        fprintf(stderr, "Could not write xml attribute\n");

        return -1;
    }

    MessengerUserLoudnessData *data;
    data = (MessengerUserLoudnessData *)message->data;
    for (int i = 0; i < message->count; i++)
    {
        ret = xmlTextWriterStartElement(writer, BAD_CAST("file"));
        if (ret < 0)
        {
            fprintf(stderr, "Could not start xml element\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("name"), "%s",
                                                data[i].name);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("record_name"),
                                                "%s", data[i].record_name);
        if (ret < 0)
        {
            fprintf(stderr, "Could not write xml attribute\n");

            return -1;
        }

        MessengerUserLoudnessSectionData *section_data;
        section_data = data[i].data;
        for (int j = 0; j < data[i].count; j++)
        {
            ret = xmlTextWriterStartElement(writer, BAD_CAST("section"));
            if (ret < 0)
            {
                fprintf(stderr, "Could not start xml element\n");

                return -1;
            }

            ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("start"),
                                                   "%s", section_data[j].start);
            if (ret < 0)
            {
                fprintf(stderr, "Could not write xml attribute\n");

                return -1;
            }

            ret = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("end"),
                                                    "%s", section_data[j].end);
            if (ret < 0)
            {
                fprintf(stderr, "Could not write xml attribute\n");

                return -1;
            }

            ret = xmlTextWriterWriteFormatAttribute(writer,
                                                    BAD_CAST("loudness"),
                                                    "%2.1f",
                                                    section_data[j].loudness);
            if (ret < 0)
            {
                fprintf(stderr, "Could not write xml attribute\n");

                return -1;
            }

            ret = xmlTextWriterWriteFormatAttribute(writer,
                                                    BAD_CAST("comment"),
                                                    "%s",
                                                    section_data[j].comment);
            if (ret < 0)
            {
                fprintf(stderr, "Could not write xml attribute\n");

                return -1;
            }

            ret = xmlTextWriterEndElement(writer);
            if (ret < 0)
            {
                fprintf(stderr, "Could not end xml element\n");

                return -1;
            }
        }

        ret = xmlTextWriterEndElement(writer);
        if (ret < 0)
        {
            fprintf(stderr, "Could not end xml element\n");

            return -1;
        }
    }

    ret = xmlTextWriterEndElement(writer);
    if (ret < 0)
    {
        fprintf(stderr, "Could not end xml element\n");

        return -1;
    }

    ret = xmlTextWriterEndDocument(writer);
    if (ret < 0)
    {
        fprintf(stderr, "Could not end xml document\n");

        return -1;
    }

    return 0;
}

static int generate_xml(MessengerMessage *message, char **buffer, int *size)
{
    int ret;

    if (!message || !buffer || !size)
    {
        return -1;
    }

    xmlBuffer *buf;
    buf = xmlBufferCreate();
    if (!buf)
    {
        fprintf(stderr, "Could not create xml buffer\n");

        xmlCleanupParser();

        return -1;
    }

    xmlTextWriter *writer;
    writer = xmlNewTextWriterMemory(buf, 0);
    if (!writer)
    {
        fprintf(stderr, "Could not get xml writer\n");

        xmlBufferFree(buf);

        xmlCleanupParser();

        return -1;
    }

    switch (message->type)
    {
        case MESSENGER_MESSAGE_TYPE_ACK:
            ret = generate_ack_xml(message, writer);
            if (ret != 0)
            {
                fprintf(stderr, "Could not generate ack xml\n");
            }
            break;

        case MESSENGER_MESSAGE_TYPE_LOUDNESS:
            ret = generate_loudness_xml(message, writer);
            if (ret != 0)
            {
                fprintf(stderr, "Could not generate loudness xml\n");
            }
            break;

        case MESSENGER_MESSAGE_TYPE_STATUS:
            ret = generate_status_xml(message, writer);
            if (ret != 0)
            {
                fprintf(stderr, "Could not generate status xml\n");
            }
            break;

        case MESSENGER_MESSAGE_TYPE_SCHEDULE:
            ret = generate_schedule_xml(message, writer);
            if (ret != 0)
            {
                fprintf(stderr, "Could not generate schedule xml\n");
            }
            break;

        case MESSENGER_MESSAGE_TYPE_PLAYBACK_LIST:
            ret = generate_playback_list_xml(message, writer);
            if (ret != 0)
            {
                fprintf(stderr, "Could not generate playback list xml\n");
            }
            break;

        case MESSENGER_MESSAGE_TYPE_LOG_LIST:
            ret = generate_log_list_xml(message, writer);
            if (ret != 0)
            {
                fprintf(stderr, "Could not generate log list xml\n");
            }
            break;

        case MESSENGER_MESSAGE_TYPE_USER_LOUDNESS:
            ret = generate_user_loudness_xml(message, writer);
            if (ret != 0)
            {
                fprintf(stderr, "Could not generate user loudness xml\n");
            }
            break;

        default:
            ret = -1;
            if (ret != 0)
            {
                fprintf(stderr, "Unknown messenger message type\n");
            }
            break;
    }

    if (ret != 0)
    {
        xmlFreeTextWriter(writer);

        xmlBufferFree(buf);

        xmlCleanupParser();

        return -1;
    }

    *buffer = (char *)malloc(buf->use);
    if (!*buffer)
    {
        fprintf(stderr, "Could not allocate xml buffer\n");

        xmlFreeTextWriter(writer);

        xmlBufferFree(buf);

        xmlCleanupParser();

        return -1;
    }

    memcpy(*buffer, buf->content, buf->use);

    *size = buf->use;

    xmlFreeTextWriter(writer);

    xmlBufferFree(buf);

    xmlCleanupParser();

    return 0;
}

static int parse_ack_xml(xmlTextReader *reader, MessengerMessage *message)
{
    int ret;

    if (!reader || !message)
    {
        return -1;
    }

    xmlReaderTypes type;
    while (1)
    {
        type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_COMMENT ||
            type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
        {
            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                return -1;
            }

            continue;
        }
        else
        {
            break;
        }
    }
    if (type != XML_READER_TYPE_ELEMENT)
    {
        fprintf(stderr, "Could not get xml element\n");

        return -1;
    }

    const xmlChar *str;
    str = xmlTextReaderName(reader);
    if (!str)
    {
        fprintf(stderr, "Could not read xml element name\n");

        return -1;
    }

    if (!xmlStrcmp(str, BAD_CAST("ack")))
    {
        message->type = MESSENGER_MESSAGE_TYPE_ACK;
    }
    else
    {
        xmlFree(BAD_CAST(str));

        return -1;
    }

    xmlFree(BAD_CAST(str));

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("ip"));
    if (str)
    {
        strncpy(message->ip, str, sizeof(message->ip));

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->ip[0] = 0;
    }

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("number"));
    if (str)
    {
        message->number = strtol(str, NULL, 10);

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->number = 0;
    }

    message->count = 0;
    message->data = NULL;

    return 0;
}

static int parse_stream_start_xml(xmlTextReader *reader,
                                  MessengerMessage *message)
{
    int ret;

    if (!reader || !message)
    {
        return -1;
    }

    xmlReaderTypes type;
    while (1)
    {
        type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_COMMENT ||
            type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
        {
            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                return -1;
            }

            continue;
        }
        else
        {
            break;
        }
    }
    if (type != XML_READER_TYPE_ELEMENT)
    {
        fprintf(stderr, "Could not get xml element\n");

        return -1;
    }

    const xmlChar *str;
    str = xmlTextReaderName(reader);
    if (!str)
    {
        fprintf(stderr, "Could not read xml element name\n");

        return -1;
    }

    if (!xmlStrcmp(str, BAD_CAST("stream_start")))
    {
        message->type = MESSENGER_MESSAGE_TYPE_STREAM_START;
    }
    else
    {
        xmlFree(BAD_CAST(str));

        return -1;
    }

    xmlFree(BAD_CAST(str));

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("ip"));
    if (str)
    {
        strncpy(message->ip, str, sizeof(message->ip));

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->ip[0] = 0;
    }

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("number"));
    if (str)
    {
        message->number = strtol(str, NULL, 10);

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->number = 0;
    }

    message->count = 0;
    message->data = NULL;

    while (1)
    {
        ret = xmlTextReaderRead(reader);
        if (ret != 1)
        {
            break;
        }

        xmlReaderTypes type;
        while (1)
        {
            type = xmlTextReaderNodeType(reader);
            if (type == XML_READER_TYPE_COMMENT ||
                type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
            {
                ret = xmlTextReaderRead(reader);
                if (ret != 1)
                {
                    fprintf(stderr, "Could not read xml\n");

                    break;
                }

                continue;
            }
            else
            {
                break;
            }
        }
        if (type != XML_READER_TYPE_ELEMENT)
        {
            break;
        }

        const xmlChar *str;
        str = xmlTextReaderName(reader);
        if (!str)
        {
            fprintf(stderr, "Could not read xml element name\n");

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        if (xmlStrcmp(str, BAD_CAST("card")))
        {
            fprintf(stderr, "Could not find xml element\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        int size;
        size = sizeof(MessengerStreamStartData) * (message->count + 1);
        message->data = (void *)realloc(message->data, size);
        if (!message->data)
        {
            fprintf(stderr, "Could not reallocate data buffer\n");

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        MessengerStreamStartData *data;
        data = (MessengerStreamStartData *)message->data;

        str = xmlTextReaderGetAttribute(reader, BAD_CAST("index"));
        if (str)
        {
            data[message->count].index = strtol(str, NULL, 10);
        }
        else
        {
            fprintf(stderr, "Could not find xml attribute\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        str = xmlTextReaderGetAttribute(reader, BAD_CAST("port"));
        if (str)
        {
            data[message->count].port = strtol(str, NULL, 10);
        }
        else
        {
            fprintf(stderr, "Could not find xml attribute\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        message->count++;
    }

    return 0;
}

static int parse_stream_stop_xml(xmlTextReader *reader,
                                 MessengerMessage *message)
{
    int ret;

    if (!reader || !message)
    {
        return -1;
    }

    xmlReaderTypes type;
    while (1)
    {
        type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_COMMENT ||
            type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
        {
            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                return -1;
            }

            continue;
        }
        else
        {
            break;
        }
    }
    if (type != XML_READER_TYPE_ELEMENT)
    {
        fprintf(stderr, "Could not get xml element\n");

        return -1;
    }

    const xmlChar *str;
    str = xmlTextReaderName(reader);
    if (!str)
    {
        fprintf(stderr, "Could not read xml element name\n");

        return -1;
    }

    if (!xmlStrcmp(str, BAD_CAST("stream_stop")))
    {
        message->type = MESSENGER_MESSAGE_TYPE_STREAM_STOP;
    }
    else
    {
        xmlFree(BAD_CAST(str));

        return -1;
    }

    xmlFree(BAD_CAST(str));

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("ip"));
    if (str)
    {
        strncpy(message->ip, str, sizeof(message->ip));

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->ip[0] = 0;
    }

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("number"));
    if (str)
    {
        message->number = strtol(str, NULL, 10);

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->number = 0;
    }

    message->count = 0;
    message->data = NULL;

    while (1)
    {
        ret = xmlTextReaderRead(reader);
        if (ret != 1)
        {
            break;
        }

        xmlReaderTypes type;
        while (1)
        {
            type = xmlTextReaderNodeType(reader);
            if (type == XML_READER_TYPE_COMMENT ||
                type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
            {
                ret = xmlTextReaderRead(reader);
                if (ret != 1)
                {
                    fprintf(stderr, "Could not read xml\n");

                    break;
                }

                continue;
            }
            else
            {
                break;
            }
        }
        if (type != XML_READER_TYPE_ELEMENT)
        {
            break;
        }

        const xmlChar *str;
        str = xmlTextReaderName(reader);
        if (!str)
        {
            fprintf(stderr, "Could not read xml element name\n");

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        if (xmlStrcmp(str, BAD_CAST("card")))
        {
            fprintf(stderr, "Could not find xml element\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        int size;
        size = sizeof(MessengerStreamStopData) * (message->count + 1);
        message->data = (void *)realloc(message->data, size);
        if (!message->data)
        {
            fprintf(stderr, "Could not reallocate data buffer\n");

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        MessengerStreamStopData *data;
        data = (MessengerStreamStopData *)message->data;

        str = xmlTextReaderGetAttribute(reader, BAD_CAST("index"));
        if (str)
        {
            data[message->count].index = strtol(str, NULL, 10);
        }
        else
        {
            fprintf(stderr, "Could not find xml attribute\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        message->count++;
    }

    return 0;
}

static int parse_loudness_start_xml(xmlTextReader *reader,
                                    MessengerMessage *message)
{
    int ret;

    if (!reader || !message)
    {
        return -1;
    }

    xmlReaderTypes type;
    while (1)
    {
        type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_COMMENT ||
            type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
        {
            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                return -1;
            }

            continue;
        }
        else
        {
            break;
        }
    }
    if (type != XML_READER_TYPE_ELEMENT)
    {
        fprintf(stderr, "Could not get xml element\n");

        return -1;
    }

    const xmlChar *str;
    str = xmlTextReaderName(reader);
    if (!str)
    {
        fprintf(stderr, "Could not read xml element name\n");

        return -1;
    }

    if (!xmlStrcmp(str, BAD_CAST("loudness_start")))
    {
        message->type = MESSENGER_MESSAGE_TYPE_LOUDNESS_START;
    }
    else
    {
        xmlFree(BAD_CAST(str));

        return -1;
    }

    xmlFree(BAD_CAST(str));

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("ip"));
    if (str)
    {
        strncpy(message->ip, str, sizeof(message->ip));

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->ip[0] = 0;
    }

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("number"));
    if (str)
    {
        message->number = strtol(str, NULL, 10);

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->number = 0;
    }

    message->count = 0;
    message->data = NULL;

    return 0;
}

static int parse_loudness_stop_xml(xmlTextReader *reader,
                                   MessengerMessage *message)
{
    int ret;

    if (!reader || !message)
    {
        return -1;
    }

    xmlReaderTypes type;
    while (1)
    {
        type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_COMMENT ||
            type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
        {
            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                return -1;
            }

            continue;
        }
        else
        {
            break;
        }
    }
    if (type != XML_READER_TYPE_ELEMENT)
    {
        fprintf(stderr, "Could not get xml element\n");

        return -1;
    }

    const xmlChar *str;
    str = xmlTextReaderName(reader);
    if (!str)
    {
        fprintf(stderr, "Could not read xml element name\n");

        return -1;
    }

    if (!xmlStrcmp(str, BAD_CAST("loudness_stop")))
    {
        message->type = MESSENGER_MESSAGE_TYPE_LOUDNESS_STOP;
    }
    else
    {
        xmlFree(BAD_CAST(str));

        return -1;
    }

    xmlFree(BAD_CAST(str));

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("ip"));
    if (str)
    {
        strncpy(message->ip, str, sizeof(message->ip));

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->ip[0] = 0;
    }

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("number"));
    if (str)
    {
        message->number = strtol(str, NULL, 10);

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->number = 0;
    }

    message->count = 0;
    message->data = NULL;

    return 0;
}

static int parse_status_start_xml(xmlTextReader *reader,
                                  MessengerMessage *message)
{
    int ret;

    if (!reader || !message)
    {
        return -1;
    }

    xmlReaderTypes type;
    while (1)
    {
        type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_COMMENT ||
            type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
        {
            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                return -1;
            }

            continue;
        }
        else
        {
            break;
        }
    }
    if (type != XML_READER_TYPE_ELEMENT)
    {
        fprintf(stderr, "Could not get xml element\n");

        return -1;
    }

    const xmlChar *str;
    str = xmlTextReaderName(reader);
    if (!str)
    {
        fprintf(stderr, "Could not read xml element name\n");

        return -1;
    }

    if (!xmlStrcmp(str, BAD_CAST("status_start")))
    {
        message->type = MESSENGER_MESSAGE_TYPE_STATUS_START;
    }
    else
    {
        xmlFree(BAD_CAST(str));

        return -1;
    }

    xmlFree(BAD_CAST(str));

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("ip"));
    if (str)
    {
        strncpy(message->ip, str, sizeof(message->ip));

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->ip[0] = 0;
    }

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("number"));
    if (str)
    {
        message->number = strtol(str, NULL, 10);

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->number = 0;
    }

    message->count = 0;
    message->data = NULL;

    return 0;
}

static int parse_status_stop_xml(xmlTextReader *reader,
                                 MessengerMessage *message)
{
    int ret;

    if (!reader || !message)
    {
        return -1;
    }

    xmlReaderTypes type;
    while (1)
    {
        type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_COMMENT ||
            type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
        {
            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                return -1;
            }

            continue;
        }
        else
        {
            break;
        }
    }
    if (type != XML_READER_TYPE_ELEMENT)
    {
        fprintf(stderr, "Could not get xml element\n");

        return -1;
    }

    const xmlChar *str;
    str = xmlTextReaderName(reader);
    if (!str)
    {
        fprintf(stderr, "Could not read xml element name\n");

        return -1;
    }

    if (!xmlStrcmp(str, BAD_CAST("status_stop")))
    {
        message->type = MESSENGER_MESSAGE_TYPE_STATUS_STOP;
    }
    else
    {
        xmlFree(BAD_CAST(str));

        return -1;
    }

    xmlFree(BAD_CAST(str));

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("ip"));
    if (str)
    {
        strncpy(message->ip, str, sizeof(message->ip));

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->ip[0] = 0;
    }

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("number"));
    if (str)
    {
        message->number = strtol(str, NULL, 10);

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->number = 0;
    }

    message->count = 0;
    message->data = NULL;

    return 0;
}

static int parse_channel_change_xml(xmlTextReader *reader,
                                    MessengerMessage *message)
{
    int ret;

    if (!reader || !message)
    {
        return -1;
    }

    xmlReaderTypes type;
    while (1)
    {
        type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_COMMENT ||
            type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
        {
            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                return -1;
            }

            continue;
        }
        else
        {
            break;
        }
    }
    if (type != XML_READER_TYPE_ELEMENT)
    {
        fprintf(stderr, "Could not get xml element\n");

        return -1;
    }

    const xmlChar *str;
    str = xmlTextReaderName(reader);
    if (!str)
    {
        fprintf(stderr, "Could not read xml element name\n");

        return -1;
    }

    if (!xmlStrcmp(str, BAD_CAST("channel_change")))
    {
        message->type = MESSENGER_MESSAGE_TYPE_CHANNEL_CHANGE;
    }
    else
    {
        xmlFree(BAD_CAST(str));

        return -1;
    }

    xmlFree(BAD_CAST(str));

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("ip"));
    if (str)
    {
        strncpy(message->ip, str, sizeof(message->ip));

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->ip[0] = 0;
    }

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("number"));
    if (str)
    {
        message->number = strtol(str, NULL, 10);

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->number = 0;
    }

    message->count = 0;
    message->data = NULL;

    while (1)
    {
        ret = xmlTextReaderRead(reader);
        if (ret != 1)
        {
            break;
        }

        xmlReaderTypes type;
        while (1)
        {
            type = xmlTextReaderNodeType(reader);
            if (type == XML_READER_TYPE_COMMENT ||
                type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
            {
                ret = xmlTextReaderRead(reader);
                if (ret != 1)
                {
                    fprintf(stderr, "Could not read xml\n");

                    break;
                }

                continue;
            }
            else
            {
                break;
            }
        }
        if (type != XML_READER_TYPE_ELEMENT)
        {
            break;
        }

        const xmlChar *str;
        str = xmlTextReaderName(reader);
        if (!str)
        {
            fprintf(stderr, "Could not read xml element name\n");

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        if (xmlStrcmp(str, BAD_CAST("card")))
        {
            fprintf(stderr, "Could not find xml element\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        int size;
        size = sizeof(MessengerChannelChangeData) * (message->count + 1);
        message->data = (void *)realloc(message->data, size);
        if (!message->data)
        {
            fprintf(stderr, "Could not reallocate data buffer\n");

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        MessengerChannelChangeData *data;
        data = (MessengerChannelChangeData *)message->data;

        str = xmlTextReaderGetAttribute(reader, BAD_CAST("index"));
        if (str)
        {
            data[message->count].index = strtol(str, NULL, 10);
        }
        else
        {
            fprintf(stderr, "Could not find xml attribute\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        str = xmlTextReaderGetAttribute(reader, BAD_CAST("channel"));
        if (str)
        {
            data[message->count].channel = strtol(str, NULL, 10);
        }
        else
        {
            fprintf(stderr, "Could not find xml attribute\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        message->count++;
    }

    return 0;
}

static int parse_loudness_reset_xml(xmlTextReader *reader,
                                    MessengerMessage *message)
{
    int ret;

    if (!reader || !message)
    {
        return -1;
    }

    xmlReaderTypes type;
    while (1)
    {
        type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_COMMENT ||
            type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
        {
            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                return -1;
            }

            continue;
        }
        else
        {
            break;
        }
    }
    if (type != XML_READER_TYPE_ELEMENT)
    {
        fprintf(stderr, "Could not get xml element\n");

        return -1;
    }

    const xmlChar *str;
    str = xmlTextReaderName(reader);
    if (!str)
    {
        fprintf(stderr, "Could not read xml element name\n");

        return -1;
    }

    if (!xmlStrcmp(str, BAD_CAST("loudness_reset")))
    {
        message->type = MESSENGER_MESSAGE_TYPE_LOUDNESS_RESET;
    }
    else
    {
        xmlFree(BAD_CAST(str));

        return -1;
    }

    xmlFree(BAD_CAST(str));

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("ip"));
    if (str)
    {
        strncpy(message->ip, str, sizeof(message->ip));

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->ip[0] = 0;
    }

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("number"));
    if (str)
    {
        message->number = strtol(str, NULL, 10);

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->number = 0;
    }

    message->count = 0;
    message->data = NULL;

    while (1)
    {
        ret = xmlTextReaderRead(reader);
        if (ret != 1)
        {
            break;
        }

        xmlReaderTypes type;
        while (1)
        {
            type = xmlTextReaderNodeType(reader);
            if (type == XML_READER_TYPE_COMMENT ||
                type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
            {
                ret = xmlTextReaderRead(reader);
                if (ret != 1)
                {
                    fprintf(stderr, "Could not read xml\n");

                    break;
                }

                continue;
            }
            else
            {
                break;
            }
        }
        if (type != XML_READER_TYPE_ELEMENT)
        {
            break;
        }

        const xmlChar *str;
        str = xmlTextReaderName(reader);
        if (!str)
        {
            fprintf(stderr, "Could not read xml element name\n");

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        if (xmlStrcmp(str, BAD_CAST("card")))
        {
            fprintf(stderr, "Could not find xml element\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        int size;
        size = sizeof(MessengerLoudnessResetData) * (message->count + 1);
        message->data = (void *)realloc(message->data, size);
        if (!message->data)
        {
            fprintf(stderr, "Could not reallocate data buffer\n");

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        MessengerLoudnessResetData *data;
        data = (MessengerLoudnessResetData *)message->data;

        str = xmlTextReaderGetAttribute(reader, BAD_CAST("index"));
        if (str)
        {
            data[message->count].index = strtol(str, NULL, 10);
        }
        else
        {
            fprintf(stderr, "Could not find xml attribute\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        message->count++;
    }

    return 0;
}

static int parse_schedule_xml(xmlTextReader *reader,
                              MessengerMessage *message)
{
    int ret;

    if (!reader || !message)
    {
        return -1;
    }

    xmlReaderTypes type;
    while (1)
    {
        type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_COMMENT ||
            type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
        {
            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                return -1;
            }

            continue;
        }
        else
        {
            break;
        }
    }
    if (type != XML_READER_TYPE_ELEMENT)
    {
        fprintf(stderr, "Could not get xml element\n");

        return -1;
    }

    const xmlChar *str;
    str = xmlTextReaderName(reader);
    if (!str)
    {
        fprintf(stderr, "Could not read xml element name\n");

        return -1;
    }

    if (!xmlStrcmp(str, BAD_CAST("schedule")))
    {
        message->type = MESSENGER_MESSAGE_TYPE_SCHEDULE;
    }
    else
    {
        xmlFree(BAD_CAST(str));

        return -1;
    }

    xmlFree(BAD_CAST(str));

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("ip"));
    if (str)
    {
        strncpy(message->ip, str, sizeof(message->ip));

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->ip[0] = 0;
    }

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("number"));
    if (str)
    {
        message->number = strtol(str, NULL, 10);

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->number = 0;
    }

    message->count = 0;
    message->data = NULL;

    while (1)
    {
        ret = xmlTextReaderRead(reader);
        if (ret != 1)
        {
            break;
        }

        xmlReaderTypes type;
        while (1)
        {
            type = xmlTextReaderNodeType(reader);
            if (type == XML_READER_TYPE_COMMENT ||
                type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
            {
                ret = xmlTextReaderRead(reader);
                if (ret != 1)
                {
                    fprintf(stderr, "Could not read xml\n");

                    break;
                }

                continue;
            }
            else
            {
                break;
            }
        }
        if (type != XML_READER_TYPE_ELEMENT)
        {
            break;
        }

        const xmlChar *str;
        str = xmlTextReaderName(reader);
        if (!str)
        {
            fprintf(stderr, "Could not read xml element name\n");

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        if (xmlStrcmp(str, BAD_CAST("card")))
        {
            fprintf(stderr, "Could not find xml element\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        int size;
        size = sizeof(MessengerScheduleData) * (message->count + 1);
        message->data = (void *)realloc(message->data, size);
        if (!message->data)
        {
            fprintf(stderr, "Could not reallocate data buffer\n");

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        MessengerScheduleData *data;
        data = (MessengerScheduleData *)message->data;

        str = xmlTextReaderGetAttribute(reader, BAD_CAST("index"));
        if (str)
        {
            data[message->count].index = strtol(str, NULL, 10);
        }
        else
        {
            fprintf(stderr, "Could not find xml attribute\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        str = xmlTextReaderGetAttribute(reader, BAD_CAST("start"));
        if (str)
        {
            strncpy(data[message->count].start, str,
                    sizeof(data[message->count].start));
        }
        else
        {
            fprintf(stderr, "Could not find xml attribute\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        str = xmlTextReaderGetAttribute(reader, BAD_CAST("end"));
        if (str)
        {
            strncpy(data[message->count].end, str,
                    sizeof(data[message->count].end));
        }
        else
        {
            fprintf(stderr, "Could not find xml attribute\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        str = xmlTextReaderGetAttribute(reader, BAD_CAST("channel"));
        if (str)
        {
            data[message->count].channel = strtol(str, NULL, 10);
        }
        else
        {
            fprintf(stderr, "Could not find xml attribute\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        message->count++;
    }

    return 0;
}

static int parse_schedule_request_xml(xmlTextReader *reader,
                                      MessengerMessage *message)
{
    int ret;

    if (!reader || !message)
    {
        return -1;
    }

    xmlReaderTypes type;
    while (1)
    {
        type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_COMMENT ||
            type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
        {
            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                return -1;
            }

            continue;
        }
        else
        {
            break;
        }
    }
    if (type != XML_READER_TYPE_ELEMENT)
    {
        fprintf(stderr, "Could not get xml element\n");

        return -1;
    }

    const xmlChar *str;
    str = xmlTextReaderName(reader);
    if (!str)
    {
        fprintf(stderr, "Could not read xml element name\n");

        return -1;
    }

    if (!xmlStrcmp(str, BAD_CAST("schedule_request")))
    {
        message->type = MESSENGER_MESSAGE_TYPE_SCHEDULE_REQUEST;
    }
    else
    {
        xmlFree(BAD_CAST(str));

        return -1;
    }

    xmlFree(BAD_CAST(str));

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("ip"));
    if (str)
    {
        strncpy(message->ip, str, sizeof(message->ip));

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->ip[0] = 0;
    }

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("number"));
    if (str)
    {
        message->number = strtol(str, NULL, 10);

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->number = 0;
    }

    message->count = 0;
    message->data = NULL;

    return 0;
}

static int parse_playback_list_request_xml(xmlTextReader *reader,
                                           MessengerMessage *message)
{
    int ret;

    if (!reader || !message)
    {
        return -1;
    }

    xmlReaderTypes type;
    while (1)
    {
        type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_COMMENT ||
            type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
        {
            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                return -1;
            }

            continue;
        }
        else
        {
            break;
        }
    }
    if (type != XML_READER_TYPE_ELEMENT)
    {
        fprintf(stderr, "Could not get xml element\n");

        return -1;
    }

    const xmlChar *str;
    str = xmlTextReaderName(reader);
    if (!str)
    {
        fprintf(stderr, "Could not read xml element name\n");

        return -1;
    }

    if (!xmlStrcmp(str, BAD_CAST("playback_list_request")))
    {
        message->type = MESSENGER_MESSAGE_TYPE_PLAYBACK_LIST_REQUEST;
    }
    else
    {
        xmlFree(BAD_CAST(str));

        return -1;
    }

    xmlFree(BAD_CAST(str));

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("ip"));
    if (str)
    {
        strncpy(message->ip, str, sizeof(message->ip));

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->ip[0] = 0;
    }

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("number"));
    if (str)
    {
        message->number = strtol(str, NULL, 10);

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->number = 0;
    }

    message->count = 0;
    message->data = NULL;

    return 0;
}

static int parse_log_list_request_xml(xmlTextReader *reader,
                                      MessengerMessage *message)
{
    int ret;

    if (!reader || !message)
    {
        return -1;
    }

    xmlReaderTypes type;
    while (1)
    {
        type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_COMMENT ||
            type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
        {
            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                return -1;
            }

            continue;
        }
        else
        {
            break;
        }
    }
    if (type != XML_READER_TYPE_ELEMENT)
    {
        fprintf(stderr, "Could not get xml element\n");

        return -1;
    }

    const xmlChar *str;
    str = xmlTextReaderName(reader);
    if (!str)
    {
        fprintf(stderr, "Could not read xml element name\n");

        return -1;
    }

    if (!xmlStrcmp(str, BAD_CAST("log_list_request")))
    {
        message->type = MESSENGER_MESSAGE_TYPE_LOG_LIST_REQUEST;
    }
    else
    {
        xmlFree(BAD_CAST(str));

        return -1;
    }

    xmlFree(BAD_CAST(str));

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("ip"));
    if (str)
    {
        strncpy(message->ip, str, sizeof(message->ip));

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->ip[0] = 0;
    }

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("number"));
    if (str)
    {
        message->number = strtol(str, NULL, 10);

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->number = 0;
    }

    message->count = 0;
    message->data = NULL;

    return 0;
}

static int parse_user_loudness_xml(xmlTextReader *reader,
                                   MessengerMessage *message)
{
    int ret;

    if (!reader || !message)
    {
        return -1;
    }

    xmlReaderTypes type;
    while (1)
    {
        type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_COMMENT ||
            type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
        {
            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                return -1;
            }

            continue;
        }
        else
        {
            break;
        }
    }
    if (type != XML_READER_TYPE_ELEMENT)
    {
        fprintf(stderr, "Could not get xml element\n");

        return -1;
    }

    const xmlChar *str;
    str = xmlTextReaderName(reader);
    if (!str)
    {
        fprintf(stderr, "Could not read xml element name\n");

        return -1;
    }

    if (!xmlStrcmp(str, BAD_CAST("user_loudness")))
    {
        message->type = MESSENGER_MESSAGE_TYPE_USER_LOUDNESS;
    }
    else
    {
        xmlFree(BAD_CAST(str));

        return -1;
    }

    xmlFree(BAD_CAST(str));

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("ip"));
    if (str)
    {
        strncpy(message->ip, str, sizeof(message->ip));

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->ip[0] = 0;
    }

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("number"));
    if (str)
    {
        message->number = strtol(str, NULL, 10);

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->number = 0;
    }

    message->count = 0;
    message->data = NULL;

    while (1)
    {
        ret = xmlTextReaderRead(reader);
        if (ret != 1)
        {
            break;
        }

        xmlReaderTypes type;
        while (1)
        {
            type = xmlTextReaderNodeType(reader);
            if (type == XML_READER_TYPE_COMMENT ||
                type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
            {
                ret = xmlTextReaderRead(reader);
                if (ret != 1)
                {
                    fprintf(stderr, "Could not read xml\n");

                    break;
                }

                continue;
            }
            else
            {
                break;
            }
        }
        if (type != XML_READER_TYPE_ELEMENT)
        {
            break;
        }

        const xmlChar *str;
        str = xmlTextReaderName(reader);
        if (!str)
        {
            fprintf(stderr, "Could not read xml element name\n");

            for (int i = 0; i < message->count && message->data; i++)
            {
                ((MessengerUserLoudnessData *)message->data)[i].count = 0;
                if (((MessengerUserLoudnessData *)message->data)[i].data)
                {
                    free(((MessengerUserLoudnessData *)message->data)[i].data);
                    ((MessengerUserLoudnessData *)message->data)[i].data = NULL;
                }
            }

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        if (xmlStrcmp(str, BAD_CAST("file")))
        {
            fprintf(stderr, "Could not find xml element\n");

            xmlFree(BAD_CAST(str));

            for (int i = 0; i < message->count && message->data; i++)
            {
                ((MessengerUserLoudnessData *)message->data)[i].count = 0;
                if (((MessengerUserLoudnessData *)message->data)[i].data)
                {
                    free(((MessengerUserLoudnessData *)message->data)[i].data);
                    ((MessengerUserLoudnessData *)message->data)[i].data = NULL;
                }
            }

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        int size;
        size = sizeof(MessengerUserLoudnessData) * (message->count + 1);
        message->data = (void *)realloc(message->data, size);
        if (!message->data)
        {
            fprintf(stderr, "Could not reallocate data buffer\n");

            for (int i = 0; i < message->count && message->data; i++)
            {
                ((MessengerUserLoudnessData *)message->data)[i].count = 0;
                if (((MessengerUserLoudnessData *)message->data)[i].data)
                {
                    free(((MessengerUserLoudnessData *)message->data)[i].data);
                    ((MessengerUserLoudnessData *)message->data)[i].data = NULL;
                }
            }

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        MessengerUserLoudnessData *data;
        data = (MessengerUserLoudnessData *)message->data;

        str = xmlTextReaderGetAttribute(reader, BAD_CAST("name"));
        if (str)
        {
            strncpy(data[message->count].name, str,
                    sizeof(data[message->count].name));
        }
        else
        {
            fprintf(stderr, "Could not find xml attribute\n");

            xmlFree(BAD_CAST(str));

            for (int i = 0; i < message->count && message->data; i++)
            {
                ((MessengerUserLoudnessData *)message->data)[i].count = 0;
                if (((MessengerUserLoudnessData *)message->data)[i].data)
                {
                    free(((MessengerUserLoudnessData *)message->data)[i].data);
                    ((MessengerUserLoudnessData *)message->data)[i].data = NULL;
                }
            }

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        str = xmlTextReaderGetAttribute(reader, BAD_CAST("record_name"));
        if (str)
        {
            strncpy(data[message->count].record_name, str,
                    sizeof(data[message->count].record_name));
        }
        else
        {
            fprintf(stderr, "Could not find xml attribute\n");

            xmlFree(BAD_CAST(str));

            for (int i = 0; i < message->count && message->data; i++)
            {
                ((MessengerUserLoudnessData *)message->data)[i].count = 0;
                if (((MessengerUserLoudnessData *)message->data)[i].data)
                {
                    free(((MessengerUserLoudnessData *)message->data)[i].data);
                    ((MessengerUserLoudnessData *)message->data)[i].data = NULL;
                }
            }

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        data[message->count].count = 0;
        data[message->count].data = NULL;

        while (1)
        {
            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                break;
            }

            xmlReaderTypes type;
            while (1)
            {
                type = xmlTextReaderNodeType(reader);
                if (type == XML_READER_TYPE_COMMENT ||
                    type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
                {
                    ret = xmlTextReaderRead(reader);
                    if (ret != 1)
                    {
                        fprintf(stderr, "Could not read xml\n");

                        break;
                    }

                    continue;
                }
                else
                {
                    break;
                }
            }
            if (type != XML_READER_TYPE_ELEMENT)
            {
                break;
            }

            const xmlChar *str;
            str = xmlTextReaderName(reader);
            if (!str)
            {
                fprintf(stderr, "Could not read xml element name\n");

                for (int i = 0; i < message->count && message->data; i++)
                {
                    ((MessengerUserLoudnessData *)message->data)[i].count = 0;
                    if (((MessengerUserLoudnessData *)message->data)[i].data)
                    {
                        free(((MessengerUserLoudnessData *)
                                                        message->data)[i].data);
                        ((MessengerUserLoudnessData *)message->data)[i].data =
                                                                           NULL;
                    }
                }

                message->count = 0;
                if (message->data)
                {
                    free(message->data);
                    message->data = NULL;
                }

                return -1;
            }

            if (xmlStrcmp(str, BAD_CAST("section")))
            {
                fprintf(stderr, "Could not find xml element\n");

                xmlFree(BAD_CAST(str));

                for (int i = 0; i < message->count && message->data; i++)
                {
                    ((MessengerUserLoudnessData *)message->data)[i].count = 0;
                    if (((MessengerUserLoudnessData *)message->data)[i].data)
                    {
                        free(((MessengerUserLoudnessData *)
                                                        message->data)[i].data);
                        ((MessengerUserLoudnessData *)message->data)[i].data =
                                                                           NULL;
                    }
                }

                message->count = 0;
                if (message->data)
                {
                    free(message->data);
                    message->data = NULL;
                }

                return -1;
            }

            xmlFree(BAD_CAST(str));

            int size;
            size = sizeof(MessengerUserLoudnessSectionData) *
                   (data[message->count].count + 1);
            data[message->count].data = (void *)realloc(
                                                      data[message->count].data,
                                                      size);
            if (!data[message->count].data)
            {
                fprintf(stderr, "Could not reallocate data buffer\n");

                for (int i = 0; i < message->count && message->data; i++)
                {
                    ((MessengerUserLoudnessData *)message->data)[i].count = 0;
                    if (((MessengerUserLoudnessData *)message->data)[i].data)
                    {
                        free(((MessengerUserLoudnessData *)
                                                        message->data)[i].data);
                        ((MessengerUserLoudnessData *)message->data)[i].data =
                                                                           NULL;
                    }
                }

                message->count = 0;
                if (message->data)
                {
                    free(message->data);
                    message->data = NULL;
                }

                return -1;
            }

            MessengerUserLoudnessSectionData *section_data;
            section_data = (MessengerUserLoudnessSectionData *)
                                                      data[message->count].data;

            str = xmlTextReaderGetAttribute(reader, BAD_CAST("start"));
            if (str)
            {
                size = sizeof(section_data[data[message->count].count].start);
                strncpy(section_data[data[message->count].count].start, str,
                        size);
            }
            else
            {
                fprintf(stderr, "Could not find xml attribute\n");

                xmlFree(BAD_CAST(str));

                for (int i = 0; i < message->count && message->data; i++)
                {
                    ((MessengerUserLoudnessData *)message->data)[i].count = 0;
                    if (((MessengerUserLoudnessData *)message->data)[i].data)
                    {
                        free(((MessengerUserLoudnessData *)
                                                        message->data)[i].data);
                        ((MessengerUserLoudnessData *)message->data)[i].data =
                                                                           NULL;
                    }
                }

                message->count = 0;
                if (message->data)
                {
                    free(message->data);
                    message->data = NULL;
                }

                return -1;
            }

            xmlFree(BAD_CAST(str));

            str = xmlTextReaderGetAttribute(reader, BAD_CAST("end"));
            if (str)
            {
                size = sizeof(section_data[data[message->count].count].end);
                strncpy(section_data[data[message->count].count].end, str,
                        size);
            }
            else
            {
                fprintf(stderr, "Could not find xml attribute\n");

                xmlFree(BAD_CAST(str));

                for (int i = 0; i < message->count && message->data; i++)
                {
                    ((MessengerUserLoudnessData *)message->data)[i].count = 0;
                    if (((MessengerUserLoudnessData *)message->data)[i].data)
                    {
                        free(((MessengerUserLoudnessData *)
                                                        message->data)[i].data);
                        ((MessengerUserLoudnessData *)message->data)[i].data =
                                                                           NULL;
                    }
                }

                message->count = 0;
                if (message->data)
                {
                    free(message->data);
                    message->data = NULL;
                }

                return -1;
            }

            xmlFree(BAD_CAST(str));

            str = xmlTextReaderGetAttribute(reader, BAD_CAST("loudness"));
            if (str)
            {
                section_data[data[message->count].count].loudness =
                                                              strtod(str, NULL);
            }
            else
            {
                fprintf(stderr, "Could not find xml attribute\n");

                xmlFree(BAD_CAST(str));

                for (int i = 0; i < message->count && message->data; i++)
                {
                    ((MessengerUserLoudnessData *)message->data)[i].count = 0;
                    if (((MessengerUserLoudnessData *)message->data)[i].data)
                    {
                        free(((MessengerUserLoudnessData *)
                                                        message->data)[i].data);
                        ((MessengerUserLoudnessData *)message->data)[i].data =
                                                                           NULL;
                    }
                }

                message->count = 0;
                if (message->data)
                {
                    free(message->data);
                    message->data = NULL;
                }

                return -1;
            }

            xmlFree(BAD_CAST(str));

            str = xmlTextReaderGetAttribute(reader, BAD_CAST("comment"));
            if (str)
            {
                size = sizeof(section_data[data[message->count].count].comment);
                strncpy(section_data[data[message->count].count].comment, str,
                        size);
            }
            else
            {
                fprintf(stderr, "Could not find xml attribute\n");

                xmlFree(BAD_CAST(str));

                for (int i = 0; i < message->count && message->data; i++)
                {
                    ((MessengerUserLoudnessData *)message->data)[i].count = 0;
                    if (((MessengerUserLoudnessData *)message->data)[i].data)
                    {
                        free(((MessengerUserLoudnessData *)
                                                        message->data)[i].data);
                        ((MessengerUserLoudnessData *)message->data)[i].data =
                                                                           NULL;
                    }
                }

                message->count = 0;
                if (message->data)
                {
                    free(message->data);
                    message->data = NULL;
                }

                return -1;
            }

            xmlFree(BAD_CAST(str));

            data[message->count].count++;
        }

        while (1)
        {
            type = xmlTextReaderNodeType(reader);
            if (type == XML_READER_TYPE_COMMENT ||
                type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
            {
                ret = xmlTextReaderRead(reader);
                if (ret != 1)
                {
                    fprintf(stderr, "Could not read xml\n");

                    break;
                }

                continue;
            }
            else
            {
                break;
            }
        }
        if (type != XML_READER_TYPE_END_ELEMENT)
        {
            fprintf(stderr, "Wrong xml end element type\n");

            for (int i = 0; i < message->count && message->data; i++)
            {
                ((MessengerUserLoudnessData *)message->data)[i].count = 0;
                if (((MessengerUserLoudnessData *)message->data)[i].data)
                {
                    free(((MessengerUserLoudnessData *)message->data)[i].data);
                    ((MessengerUserLoudnessData *)message->data)[i].data = NULL;
                }
            }

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        str = xmlTextReaderName(reader);
        if (!str)
        {
            fprintf(stderr, "Could not read xml element name\n");

            for (int i = 0; i < message->count && message->data; i++)
            {
                ((MessengerUserLoudnessData *)message->data)[i].count = 0;
                if (((MessengerUserLoudnessData *)message->data)[i].data)
                {
                    free(((MessengerUserLoudnessData *)message->data)[i].data);
                    ((MessengerUserLoudnessData *)message->data)[i].data = NULL;
                }
            }

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        if (xmlStrcmp(str, BAD_CAST("file")))
        {
            fprintf(stderr, "Could not find xml element\n");

            xmlFree(BAD_CAST(str));

            for (int i = 0; i < message->count && message->data; i++)
            {
                ((MessengerUserLoudnessData *)message->data)[i].count = 0;
                if (((MessengerUserLoudnessData *)message->data)[i].data)
                {
                    free(((MessengerUserLoudnessData *)message->data)[i].data);
                    ((MessengerUserLoudnessData *)message->data)[i].data = NULL;
                }
            }

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        message->count++;
    }

    return 0;
}

static int parse_user_loudness_request_xml(xmlTextReader *reader,
                                           MessengerMessage *message)
{
    int ret;

    if (!reader || !message)
    {
        return -1;
    }

    xmlReaderTypes type;
    while (1)
    {
        type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_COMMENT ||
            type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
        {
            ret = xmlTextReaderRead(reader);
            if (ret != 1)
            {
                fprintf(stderr, "Could not read xml\n");

                return -1;
            }

            continue;
        }
        else
        {
            break;
        }
    }
    if (type != XML_READER_TYPE_ELEMENT)
    {
        fprintf(stderr, "Could not get xml element\n");

        return -1;
    }

    const xmlChar *str;
    str = xmlTextReaderName(reader);
    if (!str)
    {
        fprintf(stderr, "Could not read xml element name\n");

        return -1;
    }

    if (!xmlStrcmp(str, BAD_CAST("user_loudness_request")))
    {
        message->type = MESSENGER_MESSAGE_TYPE_USER_LOUDNESS_REQUEST;
    }
    else
    {
        xmlFree(BAD_CAST(str));

        return -1;
    }

    xmlFree(BAD_CAST(str));

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("ip"));
    if (str)
    {
        strncpy(message->ip, str, sizeof(message->ip));

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->ip[0] = 0;
    }

    str = xmlTextReaderGetAttribute(reader, BAD_CAST("number"));
    if (str)
    {
        message->number = strtol(str, NULL, 10);

        xmlFree(BAD_CAST(str));
    }
    else
    {
        message->number = 0;
    }

    message->count = 0;
    message->data = NULL;

    while (1)
    {
        ret = xmlTextReaderRead(reader);
        if (ret != 1)
        {
            break;
        }

        xmlReaderTypes type;
        while (1)
        {
            type = xmlTextReaderNodeType(reader);
            if (type == XML_READER_TYPE_COMMENT ||
                type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
            {
                ret = xmlTextReaderRead(reader);
                if (ret != 1)
                {
                    fprintf(stderr, "Could not read xml\n");

                    break;
                }

                continue;
            }
            else
            {
                break;
            }
        }
        if (type != XML_READER_TYPE_ELEMENT)
        {
            break;
        }

        const xmlChar *str;
        str = xmlTextReaderName(reader);
        if (!str)
        {
            fprintf(stderr, "Could not read xml element name\n");

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        if (xmlStrcmp(str, BAD_CAST("file")))
        {
            fprintf(stderr, "Could not find xml element\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        int size;
        size = sizeof(MessengerUserLoudnessRequestData) * (message->count + 1);
        message->data = (void *)realloc(message->data, size);
        if (!message->data)
        {
            fprintf(stderr, "Could not reallocate data buffer\n");

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        MessengerUserLoudnessRequestData *data;
        data = (MessengerUserLoudnessRequestData *)message->data;

        str = xmlTextReaderGetAttribute(reader, BAD_CAST("name"));
        if (str)
        {
            strncpy(data[message->count].name, str,
                    sizeof(data[message->count].name));
        }
        else
        {
            fprintf(stderr, "Could not find xml attribute\n");

            xmlFree(BAD_CAST(str));

            message->count = 0;
            if (message->data)
            {
                free(message->data);
                message->data = NULL;
            }

            return -1;
        }

        xmlFree(BAD_CAST(str));

        message->count++;
    }

    return 0;
}

static int parse_xml(char *buffer, int size, MessengerMessage *message)
{
    int ret;

    if (!buffer || !message)
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

    ret = xmlTextReaderRead(reader);
    if (ret != 1)
    {
        fprintf(stderr, "Could not read xml\n");

        xmlFreeTextReader(reader);

        xmlCleanupParser();

        return -1;
    }

    ret = parse_ack_xml(reader, message);
    if (ret != 0)
    {
        ret = parse_stream_start_xml(reader, message);
    }
    if (ret != 0)
    {
        ret = parse_stream_stop_xml(reader, message);
    }
    if (ret != 0)
    {
        ret = parse_loudness_start_xml(reader, message);
    }
    if (ret != 0)
    {
        ret = parse_loudness_stop_xml(reader, message);
    }
    if (ret != 0)
    {
        ret = parse_status_start_xml(reader, message);
    }
    if (ret != 0)
    {
        ret = parse_status_stop_xml(reader, message);
    }
    if (ret != 0)
    {
        ret = parse_channel_change_xml(reader, message);
    }
    if (ret != 0)
    {
        ret = parse_loudness_reset_xml(reader, message);
    }
    if (ret != 0)
    {
        ret = parse_schedule_xml(reader, message);
    }
    if (ret != 0)
    {
        ret = parse_schedule_request_xml(reader, message);
    }
    if (ret != 0)
    {
        ret = parse_playback_list_request_xml(reader, message);
    }
    if (ret != 0)
    {
        ret = parse_log_list_request_xml(reader, message);
    }
    if (ret != 0)
    {
        ret = parse_user_loudness_xml(reader, message);
    }
    if (ret != 0)
    {
        ret = parse_user_loudness_request_xml(reader, message);
    }

    if (ret != 0)
    {
        xmlFreeTextReader(reader);

        xmlCleanupParser();

        return -1;
    }

    xmlFreeTextReader(reader);

    xmlCleanupParser();

    return 0;
}

int messenger_init(int port, int buffer_size, MessengerContext *context)
{
    int ret;

    if (!context)
    {
        return -1;
    }

    context->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (context->fd == -1)
    {
        fprintf(stderr, "Could not open socket\n");

        return -1;
    }

    ret = fcntl(context->fd, F_GETFL, 0);
    ret = fcntl(context->fd, F_SETFL, ret | O_NONBLOCK);
    if (ret == -1)
    {
        fprintf(stderr, "Could not set socket flag\n");

        close(context->fd);

        return -1;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    ret = bind(context->fd, (struct sockaddr *)&address, sizeof(address));
    if (ret == -1)
    {
        fprintf(stderr, "Could not bind socket\n");

        close(context->fd);

        return -1;
    }

    ret = listen(context->fd, 1);
    if (ret == -1)
    {
        fprintf(stderr, "Could not listen socket\n");

        close(context->fd);

        return -1;
    }

    context->rx_buffer = (char *)malloc(sizeof(char) * buffer_size);
    if (!context->rx_buffer)
    {
        fprintf(stderr, "Could not allocate messenger rx buffer\n");

        close(context->fd);

        return -1;
    }

    context->rx_buffer_index = 0;
    context->buffer_size = buffer_size;
    context->client_fd = -1;

    return 0;
}

void messenger_uninit(MessengerContext *context)
{
    if (!context)
    {
        return;
    }

    if (0 < context->client_fd)
    {
        close(context->client_fd);
    }

    free(context->rx_buffer);

    close(context->fd);
}

int messenger_send_message(MessengerContext *context, MessengerMessage *message)
{
    int ret;

    if (!context || !message)
    {
        return -1;
    }

    if (context->client_fd < 0)
    {
        context->client_fd = accept(context->fd, NULL, NULL);
    }

    if (0 < context->client_fd)
    {
        char *buffer;
        int size;
        ret = generate_xml(message, &buffer, &size);
        if (ret != 0)
        {
            fprintf(stderr, "Could not generate xml\n");

            return -1;
        }

        ret = send(context->client_fd, buffer, size,
                   MSG_NOSIGNAL | MSG_DONTWAIT);
        if (ret == -1)
        {
            if (errno == EPIPE)
            {
                fprintf(stderr, "Could not send message\n");

                close(context->client_fd);

                context->client_fd = -1;
            }

            free(buffer);

            return -1;
        }

        free(buffer);

        if (ret != size)
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    return 0;
}

int messenger_receive_message(MessengerContext *context,
                              MessengerMessage *message)
{
    int ret;

    if (!context || !message)
    {
        return -1;
    }

    if (context->client_fd < 0)
    {
        context->client_fd = accept(context->fd, NULL, NULL);
    }

    if (0 < context->client_fd)
    {
        ret = recv(context->client_fd,
                   &context->rx_buffer[context->rx_buffer_index],
                   context->buffer_size - context->rx_buffer_index,
                   MSG_NOSIGNAL | MSG_DONTWAIT);
        if (ret == -1)
        {
	   if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
            	//pass  
            } else {
                fprintf(stderr, "Could not receive message\n");

                close(context->client_fd);

                context->client_fd = -1;
            }
        }
        else if (ret == 0)
        {
            fprintf(stderr, "Could not receive message\n");

            close(context->client_fd);

            context->client_fd = -1;
        }
        else
        {
            context->rx_buffer_index += ret;
        }
    }

    char *ptr;
    ptr = memmem(context->rx_buffer, context->rx_buffer_index, XML_HEADER,
                 strlen(XML_HEADER));

    char *next_ptr;
    if (ptr)
    {
        next_ptr = memmem(&ptr[strlen(XML_HEADER)],
                          context->rx_buffer_index - strlen(XML_HEADER),
                          XML_HEADER, strlen(XML_HEADER));
    }
    else
    {
        next_ptr = NULL;
    }

    if (ptr && next_ptr)
    {
        int len;
        len = (int)(next_ptr - ptr);
        ret = parse_xml(ptr, len, message);
        if (ret != 0)
        {
            fprintf(stderr, "Could not parse xml\n");

            context->rx_buffer_index -= (int)(next_ptr - context->rx_buffer);
            memcpy(context->rx_buffer, next_ptr, context->rx_buffer_index);

            return -1;
        }

        context->rx_buffer_index -= (int)(next_ptr - context->rx_buffer);
        memcpy(context->rx_buffer, next_ptr, context->rx_buffer_index);
    }
    else if (ptr && !next_ptr)
    {
        int len;
        len = context->rx_buffer_index - (int)(ptr - context->rx_buffer);
        ret = parse_xml(ptr, len, message);
        if (ret != 0)
        {
            fprintf(stderr, "Could not parse xml\n");

            context->rx_buffer_index = 0;

            return -1;
        }

        context->rx_buffer_index = 0;
    }
    else
    {
        return -1;
    }

    return 0;
}
