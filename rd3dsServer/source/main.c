#include <stdio.h>
#include "server.h"
#include "capture.h"

int main()
{
    setbuf(stdout, NULL);  // Flush after every print.

    CaptureInit();
    ServerInit(54346);

    while (1)
    {
        ServerRun();
    }

    ServerExit();
    CaptureExit();

    return 0;
}
