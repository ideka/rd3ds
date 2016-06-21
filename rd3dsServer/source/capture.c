#include "capture.h"
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9tex.h>
#include <windows.h>
#include "packet.h"

int CaptureInit()
{
    capture.finalSurface = NULL;

    // Get window.
    capture.window = GetDesktopWindow();

    // Create interface.
    capture.d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (capture.d3d == NULL)
    {
        printf("Couldn't create D3D9 interface.\n");
        return 1;
    }

    // Get window dimensions.
    RECT windowRect;
    GetClientRect(capture.window, &windowRect);
    unsigned int windowWidth = windowRect.right - windowRect.left;
    unsigned int windowHeight = windowRect.bottom - windowRect.top;

    // Create device.
    {
        D3DPRESENT_PARAMETERS pp;
        memset(&pp, 0, sizeof(D3DPRESENT_PARAMETERS));
        pp.BackBufferWidth = windowWidth;
        pp.BackBufferHeight = windowHeight;
        //pp.BackBufferFormat = D3DFMT_R8G8B8;
        pp.SwapEffect = D3DSWAPEFFECT_COPY;
        pp.hDeviceWindow = capture.window;
        pp.Windowed = TRUE;
        HRESULT r = IDirect3D9_CreateDevice(
                capture.d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                capture.window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp,
                &capture.device);
        if (FAILED(r))
        {
            printf("Couldn't create D3D9 device (%ld).\n", r);
            return 1;
        }
    }

    IDirect3DDevice9_CreateOffscreenPlainSurface(
            capture.device, windowWidth, windowHeight, D3DFMT_A8R8G8B8,
            D3DPOOL_SCRATCH, &capture.captureSurface, NULL);

    return 0;
}

void CaptureExit()
{
#ifdef ALPHA
    if (capture.pRet != NULL)
        free(capture.pRet);
#endif
    if (capture.finalSurface != NULL)
        IDirect3D9_Release(capture.finalSurface);
    IDirect3D9_Release(capture.captureSurface);
    IDirect3D9_Release(capture.device);
    IDirect3D9_Release(capture.d3d);
}

void CaptureConfig(unsigned int width, unsigned int height, Format format,
        int filter)
{
#ifdef ALPHA
    if (capture.pRet != NULL)
        free(capture.pRet);
    capture.pRet = NULL;
#endif
    capture.finalWidth = width;
    capture.finalHeight = height;
    capture.filter = filter;

    D3DFORMAT d3dFormat;
    switch (format)
    {
        case FORMAT_R8G8B8:
            d3dFormat = D3DFMT_A8R8G8B8;
            capture.bpp = 3;
            break;

        case FORMAT_R5G6B5:
            d3dFormat = D3DFMT_R5G6B5;
            capture.bpp = 2;
            break;
    }

    if (capture.finalSurface != NULL)
        IDirect3D9_Release(capture.finalSurface);

    IDirect3DDevice9_CreateOffscreenPlainSurface(
            capture.device, capture.finalWidth, capture.finalHeight,
            d3dFormat, D3DPOOL_SCRATCH, &capture.finalSurface, NULL);
}

size_t CaptureGetFinalSize()
{
    return capture.finalWidth * capture.finalHeight * capture.bpp;
}

char* CaptureFrame()
{
    {
        HRESULT r = IDirect3DDevice9_GetFrontBufferData(
                capture.device, 0, capture.captureSurface);
        if (FAILED(r))
        {
            printf("Error geting front buffer data (%ld).\n", r);
            return NULL;
        }
    }

    {
        HRESULT r = D3DXLoadSurfaceFromSurface(
                capture.finalSurface, NULL, NULL,
                capture.captureSurface, NULL, NULL,
                capture.filter, 0);
        if (FAILED(r))
        {
            printf("Error converting surface (%ld).\n", r);
            return NULL;
        }
    }

    {
        D3DLOCKED_RECT bits;
        {
            HRESULT r = IDirect3DSurface9_LockRect(
                    capture.finalSurface, &bits, NULL, D3DLOCK_READONLY);
            if (FAILED(r))
            {
                printf("Failed to lock surface (%ld).\n", r);
                return NULL;
            }
        }

        {
            // Get ret.
            char* ret = malloc(CaptureGetFinalSize());
            char* pPixel = bits.pBits;
            for (unsigned int y = 0; y < capture.finalHeight; y++)
            {
                for (unsigned int x = 0; x < capture.finalWidth; x++)
                {
                    memcpy(
                            ret +
                            (capture.finalHeight * x +
                             capture.finalHeight - y - 1) * capture.bpp,
                            pPixel, capture.bpp);
                    pPixel += capture.bpp;
                    if (capture.bpp == 3)
                        pPixel++;
                }
            }
            IDirect3DSurface9_UnlockRect(capture.finalSurface);

#ifdef ALPHA
            // Duplicate ret to temp.
            char* temp = malloc(CaptureGetFinalSize());
            memcpy(temp, ret, CaptureGetFinalSize());

            // If there is a previous frame...
            if (capture.pRet != NULL)
            {
                char* r = ret;
                char* p = capture.pRet;
                for (unsigned int i = 0; i < CaptureGetFinalSize();
                        i += capture.bpp)
                {
                    if (memcmp(r, "\0\0", capture.bpp) == 0)
                        memset(r, 1, 1);
                    if (memcmp(r, p, capture.bpp) == 0)
                        memset(r, 0, capture.bpp);
                    r += capture.bpp;
                    p += capture.bpp;
                }
            }
            else
            {
                capture.pRet = malloc(CaptureGetFinalSize());
            }
            memcpy(capture.pRet, temp, CaptureGetFinalSize());
            free(temp);
#endif
            return ret;
        }
    }
}
