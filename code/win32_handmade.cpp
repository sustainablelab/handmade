/* ===========
 * $File: $
 * $Date: $
 * $Revision: $
 * ===========
 */

#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

// TODO(sustainablelab): This is a global for now.
global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;


// DIB -- Device Independent Bitmap
internal void
Win32ResizeDIBSection(int Width, int Height)
{

    // TODO(sustainablelab): Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.

    if (BitmapHandle)
    {
        DeleteObject(BitmapHandle);
    }
    if (!BitmapDeviceContext)
    {
        // TODO(sustainablelab): Should we recreate these under special circumstances
        BitmapDeviceContext = CreateCompatibleDC(0);
    }

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = Width;
    BitmapInfo.bmiHeader.biHeight = Height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    BitmapHandle = CreateDIBSection(
            BitmapDeviceContext,
            &BitmapInfo,
            DIB_RGB_COLORS,
            &BitmapMemory, // <---- THIS IS IT
            0, 0);
}

internal void
Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height)
{
    // Copy from buffer to Window
    StretchDIBits(DeviceContext,
            X, Y, Width, Height, // Dest - blit to
            X, Y, Width, Height, // Src - blit from
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
            Win32UpdateWindow(DeviceContext, X, Y, Width, Height);
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

    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    // Pointer to func that handles windows notifications
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClass(&WindowClass))
    {
        HWND WindowHandle = CreateWindowExA(
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
        if (WindowHandle) //
        {
            MSG Message;
            Running = true;
            while(Running)
            {
                // GetMessage returns:
                //     . 0 on quit
                //     . nonzero on other messages
                //     . -1 on error
                BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
                if (MessageResult > 0)
                {
                    // Translate keyboard messages
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                else // Window Quit or there is an error
                {
                    break;
                }
            }
        }
        else // CreateWindow fails if Handle is 0
        {
            // TODO: logging
        }
    }
    else // RegisterClassExA returns ATOM 0 if it fails
    {
        // TODO: log error
        // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexa
        // https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
        OutputDebugStringA("CreateWindow fail");
    }


    return(0);
}

