#ifndef PACKET_H
#define PACKET_H

#define PACKET_CONFIG		0
#define PACKET_GET_SCREEN	1
#define PACKET_SCREEN		2

#define FILTER_POINT		0x00000002
#define FILTER_LINEAR		0x00000003
#define FILTER_TRIANGLE		0x00000004

#define IP_STRING_SIZE		16

#define COMPRESSION
#define ALPHA

#define SIZEOFDATA(x) (sizeof(x) - sizeof(GenericPacket))

typedef enum
{
    FORMAT_R5G6B5,
    FORMAT_R8G8B8
} Format;

typedef struct
{
    unsigned int id;
    unsigned int dataSize;
    char data[];
} GenericPacket;

typedef struct
{
    unsigned int id;
    unsigned int dataSize;
    unsigned int width;
    unsigned int height;
    Format format;
    int filter;
} ConfigPacket;

typedef struct
{
    unsigned int id;
    unsigned int dataSize;
    char screen[];
} ScreenPacket;

/**
 * Return 0 if successful.
 * Return an error (>0) in case of error.
 */
int SendPacket(int, GenericPacket*, int);

/**
 * Return -2 if there is nothing to be received.
 * Return -1 in case of graceful disconnect.
 * Return 0 if successful (caller must free the passed GenericPacket).
 * Return an error (>0) in case of error.
 */
int RecvPacket(int, GenericPacket**);

/**
 * Return 0 if successful.
 * Return 1 if the connection was closed.
 */
int Read(int, void (*)(GenericPacket*));

#endif
