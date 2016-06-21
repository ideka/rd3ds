#include "packet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _3DS
#include <errno.h>
#include <sys/socket.h>
#include "debug.h"
#define IS_WOULDBLOCK(x) ((x) == EAGAIN || (x) == EWOULDBLOCK)
#define GET_ERROR() (errno)
#else
#include <winsock.h>
#define IS_WOULDBLOCK(x) ((x) == WSAEWOULDBLOCK)
#define GET_ERROR() (WSAGetLastError())
#endif

#ifdef _CITRA
// TODO: Bug the Citra devs to make it so I don't have to do this?
#undef EAGAIN
#define EAGAIN 6
#undef EWOULDBLOCK
#define EWOULDBLOCK 6

#define MAX_CITRA (0x2000 - 1)
#define MAXDATA(x) ((x) < MAX_CITRA ? (x) : MAX_CITRA)
#else
#define MAXDATA(x) (x)
#endif

int SendPacket(int fd, GenericPacket* in, int packetSize)
{
    int totalSent = 0;
    do
    {
        int r = send(
                fd, (void*)in + totalSent,
                MAXDATA(packetSize - totalSent),
                0);

        if (r > 0)
        {
            totalSent += r;
        }
        else if (!IS_WOULDBLOCK(GET_ERROR()))
        {
            printf("send error: %d\n", GET_ERROR());
            return GET_ERROR();
        }
        //printf("sent %d / %d\n", totalSent, packetSize);
    }
    while (totalSent < packetSize);

    return 0;
}

int RecvPacket(int fd, GenericPacket** out)
{
    int sizeofGenericPacket = sizeof(GenericPacket);
    GenericPacket packet;

    {
        // FIXME: Call recv multiple times here if needed.
        int r = recv(fd, (void*)&packet, sizeofGenericPacket, 0);

        if (r == 0)
        {
            return -1;
        }
        else if (r < 0)
        {
            if (IS_WOULDBLOCK(GET_ERROR()))
                return -2;
            else
                return GET_ERROR();
        }
    }

    int totalSize = sizeofGenericPacket + packet.dataSize;
    *out = malloc(totalSize);
    memcpy(*out, (char*)&packet, sizeofGenericPacket);

    if (packet.dataSize == 0)
        return 0;

    {
        int totalReceived = sizeofGenericPacket;
        do
        {
            int r = recv(
                    fd, (void*)*out + totalReceived,
                    MAXDATA(totalSize - totalReceived),
                    0);

            if (r > 0)
            {
                totalReceived += r;
            }
            else if (r == 0)
            {
                free(*out);
                return -1;
            }
            else if (!IS_WOULDBLOCK(GET_ERROR()))
            {
                free(*out);
                return GET_ERROR();
            }
            //printf("received %d / %d\n", totalReceived, totalSize);
        }
        while (totalReceived < totalSize);
    }

    return 0;
}

int Read(int fd, void (*dispatch)(GenericPacket*))
{
    GenericPacket* p;

    {
        int r = RecvPacket(fd, &p);

        if (r == -2)
            return 0;

        if (r != 0)
        {
            if (r > 0)
                printf("recv error: %d.\n", r);
            return 1;
        }
    }

    (*dispatch)(p);
    free(p);
    return 0;
}
