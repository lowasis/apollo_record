#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

static int get_line(char *buffer, int size, int *index)
{
    int ret;

    ret = getchar();
    if (ret == EOF)
    {
        return -1;
    }

    if (ret != '\n')
    {
        if (*index < size)
        {
            buffer[(*index)++] = ret;
        }

        return -1;
    }

    if (*index < size)
    {
        buffer[*index] = 0;
    }
    else
    {
        buffer[size - 1] = 0;
    }
    *index = 0;

    return 0;
}

int main(int argc, char **argv)
{
    int ret;

    if (argc != 3)
    {
        printf("Usage : ./%s [ip] [port]\n", argv[0]);
        exit(0);
    }

    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Connect error: ");

        exit(0);
    }

    ret = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, ret | O_NONBLOCK);

    char line_buf[128];
    int idx = 0;

    char buf[1024];
    int len;

    while(1)
    {
        ret = get_line(line_buf, sizeof(line_buf), &idx);
        if (ret == 0)
        {
            if (!strcmp(line_buf, "stream_start"))
            {
                len = snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!-- comment -->\n<stream_start ip=\"10.10.2.22\" number=\"235\">\n   <card index=\"0\" port=\"3401\" />\n   <card index=\"1\" port=\"3402\" />\n"
                                                 "   <card index=\"2\" port=\"3403\" />\n   <card index=\"3\" port=\"3404\" />\n   <card index=\"4\" port=\"3405\" />\n</stream_start>\n");
                send(sockfd, buf, len, 0);

                printf("stream_start sent\n");
            }
            else if (!strcmp(line_buf, "stream_stop"))
            {
                len = snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!-- comment -->\n<stream_stop ip=\"10.10.2.22\" number=\"235\">\n   <card index=\"0\" />\n   <card index=\"1\" />\n"
                                                 "   <card index=\"2\" />\n   <card index=\"3\" />\n   <card index=\"4\" />\n<!-- comment -->\n</stream_stop>\n");
                send(sockfd, buf, len, 0);

                printf("stream_stop sent\n");
            }
            if (!strcmp(line_buf, "loudness_start"))
            {
                len = snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><loudness_start ip=\"192.168.1.1\" number=\"1111\" >\n");
                send(sockfd, buf, len, 0);

                printf("loudness_start sent\n");
            }
            else if (!strcmp(line_buf, "loudness_stop"))
            {
                len = snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><loudness_stop ip=\"192.168.2.2\" number=\"2222\" />\n");
                send(sockfd, buf, len, 0);

                printf("loudness_stop sent\n");
            }
            else if (!strcmp(line_buf, "status_start"))
            {
                len = snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><status_start ip=\"192.168.3.3\" number=\"3333\" />\n");
                send(sockfd, buf, len, 0);

                printf("status_start sent\n");
            }
            else if (!strcmp(line_buf, "status_stop"))
            {
                len = snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><status_stop ip=\"192.168.4.4\" number=\"4444\" />\n");
                send(sockfd, buf, len, 0);

                printf("staus_stop sent\n");
            }
            else if (!strcmp(line_buf, "channel_change"))
            {
                len = snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><channel_change ip=\"10.10.1.20\" number=\"7777\"><card index=\"1\" channel=\"11\" /><card index=\"3\" channel=\"29\" /></channel_change>\n");
                send(sockfd, buf, len, 0);

                printf("channel_change sent\n");
            }
            else if (!strcmp(line_buf, "loudness_reset"))
            {
                len = snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><loudness_reset ip=\"10.10.1.20\" number=\"8888\">\n\n\n    <!-- 올바른 xml/xhtml 주석 --><card index=\"1\" /><card index=\"4\" /></loudness_reset>\n");
                send(sockfd, buf, len, 0);

                printf("loudness_reset sent %s \n", buf);
            }
            else if (!strcmp(line_buf, "schedule"))
            {
                len = snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><schedule ip=\"10.10.1.10\" number=\"63233\"><card index=\"1\" start=\"2018-09-18 18:40:00\" end=\"2018-09-18 18:50:00\" channel=\"11\" />"
                                                 "<card index=\"1\" start=\"2018-09-30 10:30:25\" end=\"2018-10-02 04:29:00\" channel=\"15\" />"
                                                 "<card index=\"2\" start=\"2018-09-19 00:00:00\" end=\"2018-09-19 01:00:00\" channel=\"7\" />"
                                                 "<card index=\"5\" start=\"2018-09-17 15:00:00\" end=\"2018-09-20 18:00:00\" channel=\"29\" /></schedule>\n");
                send(sockfd, buf, len, 0);

                printf("schedule sent\n");
            }
            else if (!strcmp(line_buf, "schedule_request"))
            {
                len = snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><schedule_request ip=\"10.10.1.20\" number=\"5323\" />\n");
                send(sockfd, buf, len, 0);

                printf("schedule_request sent\n");
            }
            else if (!strcmp(line_buf, "playback_list_request"))
            {
                len = snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><playback_list_request ip=\"10.10.1.20\" number=\"5323\" />\n");
                send(sockfd, buf, len, 0);

                printf("playback_list_request sent\n");
            }
            else if (!strcmp(line_buf, "log_list_request"))
            {
                len = snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><log_list_request ip=\"10.10.1.20\" number=\"5323\" />\n");
                send(sockfd, buf, len, 0);

                printf("log_list_request sent\n");
            }
            else if (!strcmp(line_buf, "user_loudness"))
            {
                len = snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><user_loudness ip=\"10.10.1.10\" number=\"65432\">"
                                                 "<file name=\"CH29_tvN_20180917_123456_윤식당_20180917_123501_1.ul\" record_name=\"CH29_tvN_20180917_123456_윤식당_20180917_123501.ts\">"
                                                 "<section start=\"00:12:22\" end=\"00:13:22\" loudness=\"-23.1\" comment=\"첫광고전\" />"
                                                 "<section start=\"00:15:11\" end=\"00:20:37\" loudness=\"-23.9\" comment=\"첫광고후~둘째광고전\" /></file>"
                                                 "<file name=\"temp_9.ul\" record_name=\"temp.ts\">"
                                                 "<section start=\"00:00:00\" end=\"00:05:00\" loudness=\"-55.0\" comment=\"AAAAAAA\" />"
                                                 "<section start=\"00:05:00\" end=\"00:10:00\" loudness=\"-44.0\" comment=\"BBB\" />"
                                                 "<section start=\"00:10:00\" end=\"00:15:00\" loudness=\"-33.0\" comment=\"CCCCCCCCCCCCCCC\" /></file>"
                                                 "</user_loudness>");
                send(sockfd, buf, len, 0);

                printf("user_loudness sent\n");
            }
            else if (!strcmp(line_buf, "user_loudness_request"))
            {
                len = snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><user_loudness_request ip=\"10.10.1.20\" number=\"3572\">"
                                                 "<file name=\"CH29_tvN_20180917_123456_윤식당_20180917_123501_1.ul\" /><file name=\"temp_9.ul\" /><file name=\"temp_8.ul\" /></user_loudness_request>");
                send(sockfd, buf, len, 0);

                printf("user_loudness_request sent\n");
            }
            else if (!strcmp(line_buf, "unknown"))
            {
                len = snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><unknown ip=\"192.168.4.4\" number=\"5555\" />\n");
                send(sockfd, buf, len, 0);

                printf("unknown sent\n");
            }
            else if (!strcmp(line_buf, "wrong"))
            {
                len = snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><wrong ip=\"192.168.4.4\" number=\"6666\" /></wrong>\n");
                send(sockfd, buf, len, 0);

                printf("wrong sent\n");
            }
        }

        ret = recv(sockfd, buf, sizeof(buf) - 1, MSG_NOSIGNAL | MSG_DONTWAIT);
        if (0 < ret)
        {
            buf[ret] = 0;

            printf("====================\n");
            printf("length : %d\n", ret);
            printf("content :\n");
            printf("%s\n", buf);
            printf("====================\n");
        }

        usleep(10 * 1000);
    }

    ret = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, ret & ~O_NONBLOCK);

    close(sockfd);

    exit(0);
}
