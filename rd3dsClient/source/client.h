#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>
#include <3ds.h>
#include "packet.h"

typedef enum
{
    CLIENTSTATE_OFF,
    CLIENTSTATE_HANDSHAKE,
    CLIENTSTATE_HANDSHAKE_WAIT,
    CLIENTSTATE_ON
} ClientState;

struct
{
    ClientState state;

    int socket;
    struct sockaddr_in serverAddr;

    u8* backBuffer;
    u8* frontBuffer;

    unsigned int bytesPerPixel;
    size_t framebufferSize;
} client;

// "Public" stuff.
int ClientInit();

void ClientExit();

void ClientRun();

void ClientSetServerInfo(char*, unsigned short);

// "Private" stuff.
int ClientOpenSocket();

void ClientCloseSocket();

int ClientConnect();

void ClientSendHandshake();

void ClientAskForScreen();

void ClientRead();

void ClientDispatch(GenericPacket*);

#endif
