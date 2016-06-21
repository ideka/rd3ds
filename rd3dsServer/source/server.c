#include "server.h"
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <zlib.h>
#include "capture.h"

int ServerInit(u_short serverPort)
{
    server.state = SERVERSTATE_OFF;
    server.serverSocket = INVALID_SOCKET;
    server.clientSocket = INVALID_SOCKET;

    printf("Initiailizing winsock.\n");
    {
        WSADATA wsa;
        int r = WSAStartup(MAKEWORD(2, 2), &wsa);
        if (r != 0)
        {
            printf("Couldn't initialize winsock (%d).\n", r);
            return r;
        }
    }

    printf("Opening socket.\n");
    server.serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (server.serverSocket == INVALID_SOCKET)
    {
        int error = WSAGetLastError();
        printf("Couldn't open socket (%d).\n", error);
        return error;
    }

    memset(&server.clientAddr, 0, sizeof(server.clientAddr));
    memset(&server.serverAddr, 0, sizeof(server.serverAddr));
    server.serverAddr.sin_family = AF_INET;
    server.serverAddr.sin_addr.s_addr = INADDR_ANY;
    server.serverAddr.sin_port = htons(serverPort);

    if (bind(
                server.serverSocket,
                (struct sockaddr*)&server.serverAddr,
                sizeof(server.serverAddr)) == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        printf("Couldn't bind socket (%d).\n", error);
        return error;
    }
    printf("Socket bound to port %d.\n", serverPort);

    // Don't block.
    {
        u_long opt = 1;
        int r = ioctlsocket(server.serverSocket, FIONBIO, &opt);
        if (r != 0)
        {
            printf("Couldn't make socket non-blocking (%d).\n", r);
            return r;
        }
    }

    listen(server.serverSocket, SOMAXCONN);

    printIPs();
    printf("Listening...\n");

    return 0;
}

void ServerExit()
{
    if (server.clientSocket != INVALID_SOCKET)
    {
        shutdown(server.clientSocket, SD_BOTH);
        closesocket(server.clientSocket);
        server.clientSocket = INVALID_SOCKET;
    }

    closesocket(server.serverSocket);
    WSACleanup();
}

void ServerRun()
{
    switch (server.state)
    {
        case SERVERSTATE_OFF:
            ServerListen();
            break;

        case SERVERSTATE_SEND_SCREEN:
            ServerSendScreen();
            ServerRead();
            break;

        case SERVERSTATE_ON:
            ServerRead();
            break;
    }
}

void ServerSendScreen()
{
    ScreenPacket* answer = malloc(
            sizeof(ScreenPacket) + CaptureGetFinalSize());
    answer->id = PACKET_SCREEN;

    Bytef* frame = (Bytef*)CaptureFrame();
    if (frame != NULL)
    {
#ifdef COMPRESSION
        uLongf srcLen = CaptureGetFinalSize();
        uLongf destLen = compressBound(srcLen);
        {
            int res = compress((Bytef*)answer->screen,
                    &destLen, frame, srcLen);
            if (res != 0)
            {
                printf("Compression error (%d).\n", res);
                free(frame);
                free(answer);
                return;
            }
        }
        answer->dataSize = destLen;
#else
        memcpy(answer->screen, frame, answer->dataSize);
#endif
        free(frame);

        printf("Sending screen.\n");
        SendPacket(
                server.clientSocket, (GenericPacket*)answer,
                sizeof(ScreenPacket) + answer->dataSize);
        server.state = SERVERSTATE_ON;
    }
    free(answer);
}

void ServerListen()
{
    {
        int c = sizeof(server.clientAddr);
        server.clientSocket = accept(
                server.serverSocket,
                (struct sockaddr*)&server.clientAddr,
                &c);
    }
    if (server.clientSocket != INVALID_SOCKET)
    {
        printf("Connection established.\n");
        server.state = SERVERSTATE_ON;
    }
}

void ServerRead()
{
    if (Read(server.clientSocket, &ServerDispatch) == 1)
    {
        printf("Connection closed.\n");
        server.state = SERVERSTATE_OFF;
    }
}

void ServerDispatch(GenericPacket* packet)
{
    switch (packet->id)
    {
        case PACKET_CONFIG:
            {
                ConfigPacket* p = (ConfigPacket*)packet;
                CaptureConfig(p->width, p->height, p->format, p->filter);
                printf("Received configuration.\n");
                SendPacket(
                        server.clientSocket, (GenericPacket*)p, sizeof(*p));
            }
            break;

        case PACKET_GET_SCREEN:
            server.state = SERVERSTATE_SEND_SCREEN;
            ServerSendScreen();
            break;

        default:
            printf("Received unknown packet id (%d).\n", packet->id);
            break;
    }
}

int printIPs()
{
    // 255 is the max length of a host name,
    // 256 includes an extra byte for NULL.
    char hostName[256];

    if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR)
    {
        printf("Couldn't get local host name (%d).\n", WSAGetLastError());
        return 1;
    }

    struct hostent *host = gethostbyname(hostName);
    if (host == 0)
    {
        printf("Host look up failure.\n");
        return 1;
    }

    printf("Addresses:\n");
    {
        struct in_addr addr;
        for (int i = 0; host->h_addr_list[i] != 0; i++)
        {
            memcpy(&addr, host->h_addr_list[i], sizeof(struct in_addr));
            printf("\t%d: %s\n", i, inet_ntoa(addr));
        }
    }

    return 0;
}
