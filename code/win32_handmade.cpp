/* ===========
 * $File: $
 * $Date: $
 * $Revision: $
 * ===========
 */

#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint32_t uint32;

// TODO(sustainablelab): This is a global for now.
global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable const int BytesPerPixel = 4;

internal void
RenderWeirdGradient(int XOffset, int YOffset)
{
    int Pitch = BitmapWidth*BytesPerPixel;
    uint8 *Row = (uint8 *)BitmapMemory;
    for(int Y = 0;
        Y < BitmapHeight;
        ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int X = 0; X < BitmapWidth; ++X)
        {
            /*
             *          byte:    0  1  2  3
             * Pixel in memory: RR GG BB XX
             * LITTLE ENDIAN ARCHITECTURE maps that like this:
             * 0x XXBBGGRR
             * Windows API creators wanted it to read like this:
             * 0x XXRRGGBB
             * Therefore the byte order is actually:
             *          byte:    0  1  2  3
             * Pixel in memory: BB GG RR XX
             * */
            uint8 Blue = (uint8)(X + XOffset);
            uint8 Green = (uint8)(Y + YOffset);
            uint8 Red = XOffset * YOffset;
            *Pixel++ = Blue | (Green << 8) | (Red << 16);
        }
        Row += Pitch;
    }
}

// DIB -- Device Independent Bitmap
internal void
Win32ResizeDIBSection(int Width, int Height)
{

    if (BitmapMemory)
    {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }
    BitmapWidth = Width;
    BitmapHeight = Height;
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    // Use negative height -> StretchDIBits creates a top-down image
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    // NOTE(sustainablelab): Thanks Chris Hecker.
    int BitmapMemorySize = (BitmapWidth*BitmapHeight)*BytesPerPixel;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    // TODO(sustainablelab): Clear to black
}

internal void
Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height)
{
    int WindowWidth = ClientRect->right - ClientRect->left;
    int WindowHeight = ClientRect->bottom - ClientRect->top;

    // Copy from buffer to Window
    StretchDIBits(DeviceContext,
            0, 0, WindowWidth, WindowHeight, // Dest - blit to
            0, 0, BitmapWidth, BitmapHeight, // Src - blit from
            BitmapMemory,
            &BitmapInfo,
            DIB_RGB_COLORS, SRCCOPY
            );
}

LRESULT CALLBACK
Win32MainWindowCallback( // "Window Procedure" that lpfnWndProc points to
    HWND Window,    // Handle to the Window
    UINT Message,   // "Window Notification" from Windows OS
    WPARAM wParam,  // Message-dependent data
    LPARAM lParam   // Message-dependent data
    )
{
    /* Default: OK (handled the message). */
    LRESULT Result = 0;

    // Use OutputDebugString() to test these cases.
    // (OutputDebugString is like a printf to a debug stream).
    // https://docs.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-outputdebugstringa
    // Interact with the window (change size, focus, click close).
    // That generates the event (Window Notification) that triggers this callback.
    switch(Message)
    {
        case WM_SIZE: // https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-size
        {
            // ClientRect is the part of the window I can draw into.
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(Width,Height);
            OutputDebugStringA("WM_SIZE\n");
        } break;

        case WM_CLOSE:
        {
            // TODO(sustainablelab): Handle this with a message to the user?
            Running = false;
        } break;

        case WM_DESTROY:
        {
            // TODO(sustainablelab): Handle this as an error - recreate window?
            Running = false;
        } break;


        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            Win32UpdateWindow(DeviceContext, &Paint.rcPaint, X, Y, Width, Height);
            EndPaint(Window, &Paint);
        } break;
        default:
        {
            /* OutputDebugStringA("default\n"); */
            // I don't care about this window event.
            // Call the Default Window Procedure.
            Result = DefWindowProcA(Window, Message, wParam, lParam);
        } break;
    }
    return(Result);
}

int CALLBACK
WinMain(
    HINSTANCE Instance,     // Handle to application instance
    HINSTANCE PrevInstance, // Handle to previous instance of this application
    LPSTR     CommandLine,  // not important
    int       ShowCode      // not important
    )
{
    // Create a WindowClass to register for creating a window.
    WNDCLASSA WindowClass = {}; // zero-initialize (all don't-care vars are 0)

    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    // Pointer to func that handles windows notifications
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClass(&WindowClass))
    {
        HWND Window = CreateWindowExA(
                                0, // DWORD dwExStyle,
                                WindowClass.lpszClassName, // LPCSTR lpClassName,
                                "Handmade Hero", // LPCSTR ,
                                WS_OVERLAPPEDWINDOW|WS_VISIBLE, // DWORD dwStyle,
                                CW_USEDEFAULT, // int X,
                                CW_USEDEFAULT, // int Y,
                                CW_USEDEFAULT, // int nWidth,
                                CW_USEDEFAULT, // int nHeight,
                                0, // HWND      hWndParent,
                                0, // HMENU     hMenu,
                                Instance, // HINSTANCE hInstance,
                                0 // LPVOID    lpParam
                                );
        if (Window) //
        {
            // Initial state of weird gradient: no offset.
            int XOffset = 0;
            int YOffset = 0;
            Running = true;
            while(Running)
            {
                // Go through the entire queue of messages.
                MSG Message;
                while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if (Message.message == WM_QUIT)
                    {
                        Running = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                // Render some image
                RenderWeirdGradient(XOffset, YOffset);

                // Blit
                HDC DeviceContext = GetDC(Window);
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                int Width = ClientRect.right - ClientRect.left;
                int Height = ClientRect.bottom - ClientRect.top;
                Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, Width, Height);
                ReleaseDC(Window, DeviceContext);

                // Animate
                ++XOffset;
                ++YOffset;
            }
        }
        else // CreateWindow fails if Handle is 0
        {
            // TODO(sustainablelab): logging
        }
    }
    else // RegisterClassExA returns ATOM 0 if it fails
    {
        // TODO(sustainablelab): log error
        // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexa
        // https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
        OutputDebugStringA("CreateWindow fail");
    }


    return(0);
}

