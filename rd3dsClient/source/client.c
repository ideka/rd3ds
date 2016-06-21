#include "client.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <3ds.h>
#include <zlib.h>
#include "config.h"

#ifdef _CITRA
// TODO: Bug the Citra devs to make it so I don't have to do this?
#undef EINPROGRESS
#define EINPROGRESS 6
#undef EISCONN
#define EISCONN 30
#endif

#define PAGE_SIZE 0x1000
#define SOC_CONTEXT_BUFFER_SIZE (PAGE_SIZE * 100)
#define NO_SOCKET -1
#define SCREEN_PIXELS (SCREEN_WIDTH * SCREEN_HEIGHT)

int ClientInit()
{
    client.state = CLIENTSTATE_OFF;
    client.socket = NO_SOCKET;

    printf("Initializing SOC.\n");
    {
        Result r = socInit(
                memalign(PAGE_SIZE, SOC_CONTEXT_BUFFER_SIZE),
                SOC_CONTEXT_BUFFER_SIZE);
        if (r != 0)
        {
            printf("Couldn't initialize SOC (%ld).\n", r);
            return 1;
        }
    }

    client.frontBuffer = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    gfxSwapBuffers();
    client.backBuffer = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);

    if (ClientOpenSocket() != 0)
        return 1;

    return 0;
}

void ClientSetServerInfo(char* ip, unsigned short port)
{
    memset(&client.serverAddr, 0, sizeof(client.serverAddr));
    client.serverAddr.sin_family = AF_INET;
    client.serverAddr.sin_addr.s_addr = inet_addr(ip);
    client.serverAddr.sin_port = htons(port);
    client.state = CLIENTSTATE_OFF;
}

void ClientExit()
{
    client.state = CLIENTSTATE_OFF;

    if (client.socket != NO_SOCKET)
        ClientCloseSocket();

    printf("Exiting SOC.\n");
    socExit();
}

void ClientRun()
{
    switch (client.state)
    {
        case CLIENTSTATE_OFF:
            ClientConnect();
            break;

        case CLIENTSTATE_HANDSHAKE:
            ClientSendHandshake();
            ClientRead();
            break;

        case CLIENTSTATE_HANDSHAKE_WAIT:
            ClientRead();
            break;

        case CLIENTSTATE_ON:
            ClientRead();
            break;
    }
}

int ClientOpenSocket()
{
    if (client.socket != NO_SOCKET)
        ClientCloseSocket();

    printf("Opening socket.\n");
    client.socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client.socket == -1)
    {
        client.socket = NO_SOCKET;
        printf("Couldn't open socket.\n");
        return 1;
    }

    // Don't block.
    {
        int r;
        {
            int flags = fcntl(client.socket, F_GETFL, 0);
            r = fcntl(client.socket, F_SETFL, flags | O_NONBLOCK);
        }

        if (r != 0)
        {
            printf("Unable to change socket flags (%d).\n", r);
            return r;
        }
    }

    return 0;
}

void ClientCloseSocket()
{
    printf("Closing socket.\n");
    shutdown(client.socket, SHUT_RDWR);
    closesocket(client.socket);
    client.socket = NO_SOCKET;
}

int ClientConnect()
{
    if (client.socket == NO_SOCKET && ClientOpenSocket() != 0)
        return errno;

    int r = connect(
            client.socket,
            (struct sockaddr*)&client.serverAddr,
            sizeof(client.serverAddr));

    if (r < 0 && errno != EISCONN)
    {
        if (errno != EINPROGRESS) 
            printf("Couldn't connect to server (%d).\n", errno);
        return errno;
    }

    printf("Connected to server.\n");
    client.state = CLIENTSTATE_HANDSHAKE;
    return 0;
}

void ClientSendHandshake()
{
    ConfigPacket p = ConfigGetPacket();

    switch (p.format)
    {
        case FORMAT_R8G8B8:
            gfxSetScreenFormat(GFX_TOP, GSP_BGR8_OES);
            client.bytesPerPixel = 3;
            break;

        case FORMAT_R5G6B5:
            gfxSetScreenFormat(GFX_TOP, GSP_RGB565_OES);
            client.bytesPerPixel = 2;
            break;
    }
    client.framebufferSize = SCREEN_PIXELS * client.bytesPerPixel;

    printf("Sending handshake.\n");
    SendPacket(client.socket, (GenericPacket*)&p, sizeof(p));
    client.state = CLIENTSTATE_HANDSHAKE_WAIT;
}

void ClientAskForScreen()
{
    GenericPacket p;
    p.id = PACKET_GET_SCREEN;
    p.dataSize = SIZEOFDATA(GenericPacket);
    SendPacket(client.socket, &p, sizeof(p));
}

void ClientHandleScreen(ScreenPacket* p)
{
    ClientAskForScreen();

    printf("Received screen.\n");
    u8* backBuffer = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    memcpy(client.backBuffer, client.frontBuffer, client.framebufferSize);
    u8* extraBuffer = malloc(client.framebufferSize);

#ifdef COMPRESSION
    {
        uLongf srcLen = client.framebufferSize;
        uncompress(extraBuffer, &srcLen, (Bytef*)p->screen, p->dataSize);
    }
#else
    memcpy(extraBuffer, p->screen, p->dataSize);
#endif

#ifdef ALPHA
    {
        u8* s = extraBuffer;
        u8* b = backBuffer;
        for (unsigned int i = 0; i < client.framebufferSize;
                i += client.bytesPerPixel)
        {
            if (memcmp(s, "\0\0", client.bytesPerPixel) != 0)
                memcpy(b, s, client.bytesPerPixel);
            b += client.bytesPerPixel;
            s += client.bytesPerPixel;
        }
    }
#else
    memcpy(backBuffer, extraBuffer, client.framebufferSize);
#endif
    free(extraBuffer);

    gfxSwapBuffers();
    u8* placeholder = client.backBuffer;
    client.backBuffer = client.frontBuffer;
    client.frontBuffer = placeholder;
}

void ClientRead()
{
    if (Read(client.socket, &ClientDispatch) == 1)
    {
        printf("Connection closed.\n");
        client.state = CLIENTSTATE_OFF;
        ClientCloseSocket();
    }
}

void ClientDispatch(GenericPacket* packet)
{
    switch (packet->id)
    {
        case PACKET_CONFIG:
            {
                ConfigPacket* p = (ConfigPacket*)packet;

                if (ConfigCheckPacket(*p))
                {
                    if (client.state == CLIENTSTATE_HANDSHAKE_WAIT)
                    {
                        printf("Received handshake.\n");
                        ClientAskForScreen();
                        client.state = CLIENTSTATE_ON;
                    }
                }
                else
                {
                    printf("Received wrong handshake.\n");
                    client.state = CLIENTSTATE_HANDSHAKE;
                }
            }
            break;

        case PACKET_SCREEN:
            if (client.state == CLIENTSTATE_ON)
                ClientHandleScreen((ScreenPacket*)packet);
            break;

        default:
            printf("Received unknown packet id (%d).\n", packet->id);
            break;
    }
}
