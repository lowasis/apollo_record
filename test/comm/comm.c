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

    char buf[256];
    int len;

    while(1)
    {
        ret = get_line(line_buf, sizeof(line_buf), &idx);
        if (ret == 0)
        {
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
