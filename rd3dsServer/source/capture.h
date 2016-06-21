#ifndef CAPTURE_H
#define CAPTURE_H

#include <d3d9.h>
#include <windows.h>
#include "packet.h"

struct {
    HWND window;
    IDirect3D9* d3d;
    IDirect3DDevice9* device;

    IDirect3DSurface9* captureSurface;
    IDirect3DSurface9* finalSurface;
#ifdef ALPHA
    char* pRet;
#endif

    unsigned int finalWidth;
    unsigned int finalHeight;
    int bpp;
    int filter;
} capture;

// "Public" stuff.
int CaptureInit();

void CaptureExit();

void CaptureConfig(unsigned int, unsigned int, Format, int);

size_t CaptureGetFinalSize();

char* CaptureFrame();

#endif
