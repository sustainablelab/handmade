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

struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

// TODO(sustainablelab): This is a global for now.
global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackBuffer;

struct win32_window_dimension
{
    int Width;
    int Height;
};

win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;

    // ClientRect is the part of the window I can draw into.
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);

    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return(Result);
}

internal void
RenderWeirdGradient(win32_offscreen_buffer Buffer, int XOffset, int YOffset)
{
    uint8 *Row = (uint8 *)Buffer.Memory;
    for(int Y = 0;
        Y < Buffer.Height;
        ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int X = 0; X < Buffer.Width; ++X)
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
        Row += Buffer.Pitch;
    }
}

// DIB -- Device Independent Bitmap
internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{

    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4; // xxRRGGBB

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    // Use negative height -> StretchDIBits creates a top-down image
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    // NOTE(sustainablelab): Thanks Chris Hecker for eliminating
    // the need to make a DC (Device Context).
    int BitmapMemorySize = ( Buffer->Width * Buffer->Height ) * Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    Buffer->Pitch = Buffer->Width * Buffer->BytesPerPixel;
    // TODO(sustainablelab): Clear to black
}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext,
                        int WindowWidth, int WindowHeight,
                        win32_offscreen_buffer Buffer,
                        int X, int Y, int Width, int Height)
{

    // Copy from buffer to Window
    StretchDIBits(DeviceContext,
            0, 0, WindowWidth, WindowHeight, // Dest - blit to
            0, 0, Buffer.Width, Buffer.Height, // Src - blit from
            Buffer.Memory,
            &(Buffer.Info),
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
            Win32DisplayBufferInWindow(DeviceContext, Width, Height, // Paint.rcPaint,
                                       GlobalBackBuffer,
                                       X, Y, Width, Height);
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
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            /* Win32ResizeDIBSection(&GlobalBackBuffer, Dimension.Width, Dimension.Height); */
            Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

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
                RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);

                // Blit
                HDC DeviceContext = GetDC(Window);

                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext,
                                           Dimension.Width, Dimension.Height,
                                           GlobalBackBuffer,
                                           0, 0, Dimension.Width, Dimension.Height);
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

