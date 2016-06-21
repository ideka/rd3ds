#ifndef PROGRAM_H
#define PROGRAM_H

#include <3ds.h>

typedef enum
{
    PROGRAMSTATE_MENU,
    PROGRAMSTATE_CLIENT,
    PROGRAMSTATE_QUIT
} ProgramState;

struct
{
    ProgramState state;

    u32 kDown;
    u32 kHeld;
    u32 kUp;
} program;

void ProgramRun();

#endif
