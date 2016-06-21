#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include "packet.h"

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

struct
{
    char serverIP[IP_STRING_SIZE];
    int serverPort;

    Format imageFormat;
    int imageFilter;
} config;

void ConfigLoad();

ConfigPacket ConfigGetPacket();

bool ConfigCheckPacket(ConfigPacket);

#endif
