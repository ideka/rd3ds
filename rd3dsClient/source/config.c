#include "config.h"
#include <stdbool.h>
#include <string.h>
#include "packet.h"

void ConfigLoad()
{
    strcpy(config.serverIP, "192.168.1.108");
    config.serverPort = 54346;
    config.imageFormat = FORMAT_R5G6B5;
    config.imageFilter = FILTER_LINEAR;
}

ConfigPacket ConfigGetPacket()
{
    ConfigPacket p;
    p.id = PACKET_CONFIG;
    p.dataSize = SIZEOFDATA(ConfigPacket);
    p.width = SCREEN_WIDTH;
    p.height = SCREEN_HEIGHT;
    p.format = config.imageFormat;
    p.filter = config.imageFilter;
    return p;
}

bool ConfigCheckPacket(ConfigPacket p)
{
    ConfigPacket q = ConfigGetPacket();
    return p.id == q.id && p.dataSize == q.dataSize && p.width == q.width &&
        p.height == q.height && p.format == q.format && p.filter == q.filter;
}
