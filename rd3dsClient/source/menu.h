#ifndef MENU_H
#define MENU_H

#include <3ds.h>

#define TOTAL_ITEMS 4

typedef enum
{
    MENUSTATE_MAIN,
    MENUSTATE_IP,
    MENUSTATE_PORT
} MenuState;

typedef struct
{
    void (*print)();
    void (*onLeft)();
    void (*onRight)();
    void (*onA)();
} MenuItem;

struct
{
    MenuState state;
    int selected;
    MenuItem items[TOTAL_ITEMS];

    short displayIndex;
    union
    {
        u8 ip[4];  // 4 bytes per IP.
        unsigned short port;
    } display;
} menu;

// "Public" stuff.
void MenuLoad();

void MenuRun();

// "Private" stuff.
void MenuRunItems();

void MenuEnterIP();

void MenuEnterPort();

void MenuRunItem(int);

// IP stuff.
void MenuIPPrint();

void MenuIPInteract();

// Port stuff.
void MenuPortPrint();

void MenuPortInteract();

// Image format stuff.
void MenuImageFormatPrint();

void MenuImageFormatOnLeftOrRight();

// Image filter stuff.
void MenuImageFilterPrint();

void MenuImageFilterOnLeft();

void MenuImageFilterOnRight();

// Other stuff.
void callIfNotNULL(void (*)());

#endif
