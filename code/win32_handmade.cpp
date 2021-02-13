/* ===========
 * $File: $
 * $Date: $
 * $Revision: $
 * ===========
 */

#include <windows.h>

LRESULT CALLBACK
MainWindowCallback( // "Window Procedure" that lpfnWndProc points to
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
            OutputDebugStringA("WM_SIZE\n");
        } break;

        case WM_DESTROY:
        {
            OutputDebugStringA("WM_DESTROY\n");
        } break;

        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE\n");
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int x = Paint.rcPaint.left;
            int y = Paint.rcPaint.top;
            int w = Paint.rcPaint.right - Paint.rcPaint.left;
            int h = Paint.rcPaint.bottom - Paint.rcPaint.top;
            DWORD Operation = BLACKNESS;
            PatBlt(DeviceContext, x, y, w, h, Operation);
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
    WindowClass.lpfnWndProc = MainWindowCallback;
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
            for(;;)
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

