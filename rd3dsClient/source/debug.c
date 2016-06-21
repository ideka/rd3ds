#include <3ds.h>
#include <stdio.h>
#include <stdarg.h>

void pause()
{
    while (1)
    {
        hidScanInput();

        u32 kDown = hidKeysDown();

        if (kDown & KEY_START)
            break;

        gspWaitForVBlank();
    }
}
