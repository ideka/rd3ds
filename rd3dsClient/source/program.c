#include "program.h"
#include <3ds.h>
#include <stdio.h>
#include "config.h"
#include "menu.h"
#include "client.h"

void ProgramRun()
{
    // Initialize stuff.
    hidInit();
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);

    ConfigLoad();
    MenuLoad();
    ClientInit();
    ClientSetServerInfo(config.serverIP, config.serverPort);

    program.state = PROGRAMSTATE_MENU;
    consoleClear();

    // Loop.
    while (aptMainLoop() && program.state != PROGRAMSTATE_QUIT)
    {
        hidScanInput();
        program.kDown = hidKeysDown();
        program.kHeld = hidKeysHeld();
        program.kUp = hidKeysUp();

        switch (program.state)
        {
            case PROGRAMSTATE_MENU:
                MenuRun();
                break;

            case PROGRAMSTATE_CLIENT:
                if (program.kHeld & KEY_START && program.kHeld & KEY_SELECT)
                {
                    program.state = PROGRAMSTATE_MENU;
                    consoleClear();
                }
                else
                {
                    ClientRun();
                }
                break;

            case PROGRAMSTATE_QUIT:
                break;
        }

        gspWaitForVBlank();
    }

    // Shut stuff down.
    ClientExit();

    gfxExit();
    hidExit();
}
