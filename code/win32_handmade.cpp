/* ===========
 * $File: $
 * $Date: $
 * $Revision: $
 * ===========
 */

#include <windows.h>

int WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd
    )
{
    MessageBox(
        0, // handle to window
        "Bob Text", // text in window
        "Bob Title", // title of window
        MB_OK | MB_ICONINFORMATION // flags
        );
    return 0;
}

