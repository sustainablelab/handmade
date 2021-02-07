/* ===========
 * $File: $
 * $Date: $
 * $Revision: $
 * ===========
 */

#include <windows.h>

LRESULT CALLBACK
MainWindowCallback(
    HWND Window, // handle to the Window
    UINT Message, // https://docs.microsoft.com/en-us/windows/win32/winmsg/window-notifications
    WPARAM wParam,
    LPARAM lParam
    )
{
    /* Default: the Message from Windows was handled OK. */
    LRESULT Result = 0;

    // Use OutputDebugString() to test these cases.
    // https://docs.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-outputdebugstringa
    // Interact with the window to generate the event.
    // That triggers this callback.
    // OutputDebugString tells the debugger to break.
    // Continue debugger execution.
    // Repeat, but interacting to test a different case.
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
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR     CommandLine,
    int       ShowCode
    )
{
    WNDCLASSA WindowClass = {}; // initialize all to 0

    /* https://docs.microsoft.com/en-us/windows/win32/winmsg/window-class-styles */
    // TODO: check if HREDRAW, VREDRAW, OWNDC still matter
    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;


    // Define window procedure: handle msg from Windows
    /* https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms633573(v=vs.85) */
    WindowClass.lpfnWndProc = MainWindowCallback;

    WindowClass.hInstance = Instance;
    /* WindowClass.hIcon = ; */
    /* WindowClass.hIconSm = ; */
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    /* if (RegisterClassExA(&WindowClass)) */
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
                BOOL MessageResult = GetMessage(
                      &Message, // LPMSG lpMsg,
                      0, // HWND  hWnd,
                      0, // UINT  wMsgFilterMin,
                      0  // UINT  wMsgFilterMax
                      );
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

