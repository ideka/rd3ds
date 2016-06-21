#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include "packet.h"

typedef enum
{
    SERVERSTATE_OFF,
    SERVERSTATE_SEND_SCREEN,
    SERVERSTATE_ON
} ServerState;

struct
{
    ServerState state;

    SOCKET serverSocket;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
} server;

// "Public" stuff.
int ServerInit(u_short);

void ServerExit();

void ServerRun();

// "Private" stuff.
void ServerSendScreen();

void ServerListen();

void ServerRead();

void ServerDispatch(GenericPacket*);

// Extra stuff.
int printIPs();

#endif
