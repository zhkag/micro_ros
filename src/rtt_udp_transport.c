#include <rtthread.h>

#if defined MICRO_ROS_USE_UDP

#include "micro_ros_rtt.h"
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>

#define DBG_SECTION_NAME  "micro_ros_serial"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

int sock;
struct hostent *host;
struct sockaddr_in server_addr;

int clock_gettime(clockid_t unused, struct timespec *tp)
{
    (void)unused;

    uint64_t m = rt_tick_get() * 1000 / RT_TICK_PER_SECOND * 1000;
    tp->tv_sec = m / 1000000;
    tp->tv_nsec = (m % 1000000) * 1000;

    return 0;
}

bool rtt_udp_transport_open(struct uxrCustomTransport * transport)
{
    struct micro_ros_agent_locator* locator = (struct micro_ros_agent_locator *) transport->args;

    host = (struct hostent *) gethostbyname( locator->address );

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        LOG_E("Socket error\n");
        closesocket(sock);
        return 0;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(locator->port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    return 1;
}

bool rtt_udp_transport_close(struct uxrCustomTransport * transport)
{
    if (sock >= 0)
    {
        closesocket(sock);
    }
    return 1;
}

size_t rtt_udp_transport_write(struct uxrCustomTransport * transport, const uint8_t *buf, size_t len, uint8_t *errcode)
{
    return sendto(sock, buf, len, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
}

size_t rtt_udp_transport_read(struct uxrCustomTransport * transport, uint8_t *buf, size_t len, int timeout, uint8_t *errcode)
{
    return recvfrom(sock, buf, len, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
}

#endif // MICRO_ROS_USE_UDP
