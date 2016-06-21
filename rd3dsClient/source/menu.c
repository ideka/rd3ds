#include "menu.h"
#include <stdio.h>
#include <3ds.h>
#include <math.h>
#include "program.h"
#include "config.h"
#include "client.h"
#include "packet.h"

#define IP_DIGITS 12
#define PORT_DIGITS 5

// "Public" stuff.
void MenuLoad()
{
    menu.state = MENUSTATE_MAIN;
    consoleClear();
    menu.selected = 0;
    menu.items[0] = (MenuItem)
    {
        .print = &MenuIPPrint,
        .onLeft = &MenuIPInteract,
        .onRight = &MenuIPInteract,
        .onA = &MenuIPInteract
    };
    menu.items[1] = (MenuItem)
    {
        .print = &MenuPortPrint,
        .onLeft = &MenuPortInteract,
        .onRight = &MenuPortInteract,
        .onA = &MenuPortInteract
    };
    menu.items[2] = (MenuItem)
    {
        .print = &MenuImageFormatPrint,
        .onLeft = &MenuImageFormatOnLeftOrRight,
        .onRight = &MenuImageFormatOnLeftOrRight,
    };
    menu.items[3] = (MenuItem)
    {
        .print = &MenuImageFilterPrint,
        .onLeft = &MenuImageFilterOnLeft,
        .onRight = &MenuImageFilterOnRight
    };
}

void MenuRun()
{
    switch (menu.state)
    {
        case MENUSTATE_MAIN:
            MenuRunItems();
            break;
        case MENUSTATE_IP:
            MenuEnterIP();
            break;
        case MENUSTATE_PORT:
            MenuEnterPort();
            break;
    }
}

// "Private" stuff.
void MenuRunItems()
{
    if (program.kDown & KEY_UP)
    {
        if (menu.selected == 0)
            menu.selected = TOTAL_ITEMS - 1;
        else
            menu.selected--;
    }

    if (program.kDown & KEY_DOWN)
    {
        if (menu.selected == TOTAL_ITEMS - 1)
            menu.selected = 0;
        else
            menu.selected++;
    }

    printf("\x1b[0;1H");
    for (int i = 0; i < TOTAL_ITEMS; i++)
        MenuRunItem(i);

    printf("\x1b[22;1HPress START to apply changes.\n");
    if (program.kDown & KEY_START)
    {
        ClientSetServerInfo(config.serverIP, config.serverPort);
        program.state = PROGRAMSTATE_CLIENT;
    }

    printf("\x1b[23;1HTouch the bottom screen to quit.\n");
    if (program.kDown & KEY_TOUCH)
        program.state = PROGRAMSTATE_QUIT;

    if (program.kDown & KEY_LEFT)
        callIfNotNULL(menu.items[menu.selected].onLeft);
    if (program.kDown & KEY_RIGHT)
        callIfNotNULL(menu.items[menu.selected].onRight);
    if (program.kDown & KEY_A)
        callIfNotNULL(menu.items[menu.selected].onA);

}

void MenuEnterIP()
{
    // Input.
    if (program.kDown & KEY_A || program.kDown & KEY_START)
    {
        snprintf(config.serverIP, IP_STRING_SIZE, "%03u.%03u.%03u.%03u",
            menu.display.ip[0], menu.display.ip[1],
            menu.display.ip[2], menu.display.ip[3]);
        menu.state = MENUSTATE_MAIN;
        consoleClear();
        return;
    }
    else if (program.kDown & KEY_B)
    {
        menu.state = MENUSTATE_MAIN;
        consoleClear();
        return;
    }

    if (program.kDown & KEY_LEFT)
    {
        if (menu.displayIndex == 0)
            menu.displayIndex = IP_DIGITS - 1;
        else
            menu.displayIndex--;
    }
    else if (program.kDown & KEY_RIGHT)
    {
        if (menu.displayIndex == IP_DIGITS - 1)
            menu.displayIndex = 0;
        else
            menu.displayIndex++;
    }

    if (program.kDown & KEY_UP)
    {
        menu.display.ip[menu.displayIndex / 3] +=
            pow(10, 2 - menu.displayIndex % 3);
    }
    else if (program.kDown & KEY_DOWN)
    {
        menu.display.ip[menu.displayIndex / 3] -=
            pow(10, 2 - menu.displayIndex % 3);
    }

    // Display.
    printf("\x1b[1;1H");
    printf("%03u.%03u.%03u.%03u\n",
            menu.display.ip[0], menu.display.ip[1],
            menu.display.ip[2], menu.display.ip[3]);
    for (int i = 0; i < IP_DIGITS; i++)
    {
        if (i % 3 == 0)
        {
            printf(" ");
        }
        if (menu.displayIndex == i)
            printf("^");
        else
            printf(" ");
    }
}

void MenuEnterPort()
{
    // Input.
    if (program.kDown & KEY_A || program.kDown & KEY_START)
    {
        config.serverPort = menu.display.port;
        menu.state = MENUSTATE_MAIN;
        consoleClear();
        return;
    }
    else if (program.kDown & KEY_B)
    {
        menu.state = MENUSTATE_MAIN;
        consoleClear();
        return;
    }

    if (program.kDown & KEY_LEFT)
    {
        if (menu.displayIndex == 0)
            menu.displayIndex = PORT_DIGITS - 1;
        else
            menu.displayIndex--;
    }
    else if (program.kDown & KEY_RIGHT)
    {
        if (menu.displayIndex == PORT_DIGITS - 1)
            menu.displayIndex = 0;
        else
            menu.displayIndex++;
    }

    if (program.kDown & KEY_UP)
    {
        menu.display.port += pow(10, 4 - menu.displayIndex % 5);
    }
    else if (program.kDown & KEY_DOWN)
    {
        menu.display.port -= pow(10, 4 - menu.displayIndex % 5);
    }

    // Display.
    printf("\x1b[1;1H");
    printf("%05d\n", menu.display.port);
    printf(" ");
    for (int i = 0; i < PORT_DIGITS; i++)
    {
        if (menu.displayIndex == i)
            printf("^");
        else
            printf(" ");
    }
}

void MenuRunItem(int i)
{
    // Print.
    printf("\n");
    if (menu.selected == i)
        printf(">");
    else
        printf(" ");
    callIfNotNULL(menu.items[i].print);
    printf("\n");
}

// IP stuff.
void MenuIPPrint()
{
    printf("IP: %s", config.serverIP);
}

void MenuIPInteract()
{
    // TODO: Find a better way to change a string IP into bytes that
    // actually works.
    {
        int a, b, c, d;
        sscanf(config.serverIP, "%d.%d.%d.%d", &a, &b, &c, &d);
        menu.display.ip[0] = a;
        menu.display.ip[1] = b;
        menu.display.ip[2] = c;
        menu.display.ip[3] = d;
    }
    menu.displayIndex = IP_DIGITS - 1;
    menu.state = MENUSTATE_IP;
    consoleClear();
}

// Port stuff.
void MenuPortPrint()
{
    printf("Port: %d", config.serverPort);
}

void MenuPortInteract()
{
    menu.display.port = config.serverPort;
    menu.displayIndex = PORT_DIGITS - 1;
    menu.state = MENUSTATE_PORT;
    consoleClear();
}

// Image format stuff.
void MenuImageFormatPrint()
{
    printf("Image format: ");
    switch (config.imageFormat)
    {
        case FORMAT_R5G6B5:
            printf("16bpp");
            break;
        case FORMAT_R8G8B8:
            printf("24bpp");
            break;
        default:
            printf("???");
            break;
    }
}

void MenuImageFormatOnLeftOrRight()
{
    switch (config.imageFormat)
    {
        default:  // Fall through;
        case FORMAT_R5G6B5:
            config.imageFormat = FORMAT_R8G8B8;
            break;
        case FORMAT_R8G8B8:
            config.imageFormat = FORMAT_R5G6B5;
            break;
    }
}

// Image filter stuff.
void MenuImageFilterPrint()
{
    printf("Image filter: ");
    switch (config.imageFilter)
    {
        // All options must be the same size.
        case FILTER_POINT:
            printf("point   ");
            break;
        case FILTER_LINEAR:
            printf("linear  ");
            break;
        case FILTER_TRIANGLE:
            printf("triangle");
            break;
        default:
            printf("???");
            break;
    }
}

void MenuImageFilterOnLeft()
{
    switch (config.imageFilter)
    {
        case FILTER_POINT:
            config.imageFilter = FILTER_TRIANGLE;
            break;
        default: // Fall through.
        case FILTER_LINEAR:
            config.imageFilter = FILTER_POINT;
            break;
        case FILTER_TRIANGLE:
            config.imageFilter = FILTER_LINEAR;
            break;
    }
}

void MenuImageFilterOnRight()
{
    switch (config.imageFilter)
    {
        case FILTER_POINT:
            config.imageFilter = FILTER_LINEAR;
            break;
        default:
        case FILTER_LINEAR:
            config.imageFilter = FILTER_TRIANGLE;
            break;
        case FILTER_TRIANGLE:
            config.imageFilter = FILTER_POINT;
            break;
    }
}

// Other stuff.
void callIfNotNULL(void (*func)())
{
    if (func != NULL)
        (*func)();
}
