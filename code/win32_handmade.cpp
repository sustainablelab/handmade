/* ===========
 * $File: $
 * $Date: $
 * $Revision: $
 * ===========
 */

#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>

typedef uint8_t uint8;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef int32_t bool32;

#define internal static
#define local_persist static
#define global_variable static

// --- Stubs for XInput (controllers) ---
//
/* Linking against xinput.lib *requires* player to have the DLLs for XBox360
 * controllers. But I want to make the controller optional.
 * So I do *not* link against xinput.lib. I do the following instead.
 * 
 * Define macros that generate signatures for our xinput funcs.
 * Then use those macros to define:
 * - types for our xinput funcs
 * - stubs for our xinput funcs
 * - use the types to define a pointer that points to the stub
 * - and finally a macro that prevents accidentally using the Windows version.
 * Now these xinput.lib calls default to stubs and the code builds without
 * linking against xinput.lib.
 */
// --- Stub XInputGetState ---
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// --- Stubs XInputSetState ---
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

// --- Stubs for DirectSound (audio hardware) ---
//
// --- Stub DirectSoundCreate ---
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);
DIRECT_SOUND_CREATE(DirectSoundCreateStub)
{
    return(DSERR_GENERIC);
}
global_variable direct_sound_create *DirectSoundCreate_ = DirectSoundCreateStub;
#define DirectSoundCreate DirectSoundCreate_

internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize) // Setup audio hardware
{
    // Load the DirectSound library.
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if (DSoundLibrary) //  Set up audio. If DirectSound unavailable, do nothing.
    {
        // TODO(sustainablelab): load API funcs from .dll
        DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");

        // NOTE(sustainablelab): Get a DirectSound object!
        // --- DirectSound uses the Component Object Model (COM) disaster.
        // Create "interface" IDirectSound using "DirectSoundCreate".
        // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/mt708922(v=vs.85)
        IDirectSound *DirectSound;
        if (SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            // Primary and Secondary buffers use the same WaveFormat.
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag      = WAVE_FORMAT_PCM; // PCM audio format
            WaveFormat.nChannels       = 2; // stereo is 2 channels
            WaveFormat.nSamplesPerSec  = SamplesPerSecond; // e.g., 46kHz
            WaveFormat.wBitsPerSample  = 16; // bits per sample of mono data
            WaveFormat.nBlockAlign     = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = SamplesPerSecond*WaveFormat.nBlockAlign;
            WaveFormat.cbSize          = 0; // nbytes of extra infromation

            // NOTE(docs.microsoft):
            // The application must call the IDirectSound::SetCooperativeLevel
            // method immediately after creating a DirectSound object.
            if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                // Use the "interface" to create DirectSound objects.

                // NOTE(sustainablelab): Create a primary buffer
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                BufferDescription.guid3DAlgorithm = DS3DALG_DEFAULT;
                IDirectSoundBuffer *PrimaryBuffer;
                HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0);
                if (SUCCEEDED(Error))
                {
                    HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
                    if (SUCCEEDED(Error))
                    {
                    }
                    else
                    {
                        // TODO(sustainablelab): Diagnostic
                        OutputDebugStringA("Cannot set primary buffer audio format");
                    }
                }
                else
                {
                    // TODO(sustainablelab): Diagnostic 
                    OutputDebugStringA("Cannot create sound buffer");
                }
            }
            else
            {
                // TODO(sustainablelab): Diagnostic
                OutputDebugStringA("SetCooperativeLevel fail");
            }
            // NOTE(sustainablelab): Create a secondary buffer
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            BufferDescription.guid3DAlgorithm = DS3DALG_DEFAULT;
            IDirectSoundBuffer *SecondaryBuffer;
            if (SUCCEEDED(DirectSound->CreateSoundBuffer(
                        &BufferDescription, &SecondaryBuffer, 0)))
            {
            }
            else
            {
                // TODO(sustainablelab): Diagnostic
                OutputDebugStringA("Cannot create secondary buffer");
            }

        }
        else
        {
            // TODO(sustainablelab): Diagnostic
            OutputDebugStringA("DirectSoundCreate fail");
        }
    }
    else
    {
        // TODO(sustainablelab): Diagnostic
        OutputDebugStringA("DirectSound dll unavailable");
    }
    // NOTE(sustainablelab): Start it playing
    // SecondaryBuffer-> ;
}

internal void
Win32LoadXInput(void) // Try to get XInput, use stubs if no XInput.
{
    // TODO(sustainablelab): Test this on Windows 8
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if (!XInputLibrary) // Try 1.3 if 1.4 is not available.
    {
        // TODO(sustainablelab): Diagnostic (which version of dll is loaded)
        HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    if (XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
    else
    {
        // TODO(sustainablelab): Diagnostic (xinput dll not available)
    }
}

struct win32_offscreen_buffer // bitmap memory
{
    // NOTE(sustainablelab):
    //     --- Pixels are always 32 bits wide, little endian ---
    //                      0x xx RR GG BB
    //     --- Memory order as viewed in VisualStudio ---
    //         BB GG RR XX  BB GG RR XX  BB GG RR XX  BB GG RR XX
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

global_variable win32_offscreen_buffer GlobalBackBuffer; // bitmap buffer
// TODO(sustainablelab): This is a global for now.
global_variable bool GlobalRunning;

struct win32_window_dimension // Width, Height
{
    int Width;
    int Height;
};

internal win32_window_dimension
Win32GetWindowDimension( // -> win32_window_dimension
    HWND Window
    )
{
    win32_window_dimension Result;

    // ClientRect is the part of the window I can draw into.
    // This is NOT the window size.
    // e.g., 1280x720 window has a 1004x516 drawing area.
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);

    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return(Result);
}

internal void
RenderWeirdGradient( // Put weird art in the bitmap buffer
    win32_offscreen_buffer *Buffer,
    int XOffset,
    int YOffset
    )
{
    uint8 *Row = (uint8 *)Buffer->Memory;
    for(int Y = 0;
        Y < Buffer->Height;
        ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int X = 0; X < Buffer->Width; ++X)
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
            /* uint8 Red = XOffset * YOffset; */
            uint8 Red = 0;
            *Pixel++ = (Red << 16) | (Green << 8) | Blue;
        }
        Row += Buffer->Pitch;
    }
}

// DIB -- Device Independent Bitmap
internal void
Win32ResizeDIBSection( // Alloc mem for bitmap buffer based on window size
    win32_offscreen_buffer *Buffer,
    int Width,
    int Height
    )
{

    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
    Buffer->Width = Width;
    Buffer->Height = Height;
    int BytesPerPixel = 4; // xxRRGGBB

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    // Use negative height -> StretchDIBits creates a top-down image
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    // NOTE(sustainablelab): Thanks Chris Hecker for eliminating
    // the need to make a DC (Device Context).
    int BitmapMemorySize = ( Buffer->Width * Buffer->Height ) * BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    Buffer->Pitch = Buffer->Width * BytesPerPixel;
    // TODO(sustainablelab): Clear to black
}

internal void
Win32DisplayBufferInWindow( // Blit the art in the bitmap buffer
    HDC DeviceContext,
    int WindowWidth, int WindowHeight,
    win32_offscreen_buffer *Buffer
    )
{

    // TODO(sustainablelab): Aspect ratio correction.
    // TODO(sustainablelab): Play with stretch modes.

    // Copy from buffer to Window
    StretchDIBits(DeviceContext,
            0, 0, WindowWidth, WindowHeight, // Dest - blit to
            0, 0, Buffer->Width, Buffer->Height, // Src - blit from
            Buffer->Memory,
            &Buffer->Info,
            DIB_RGB_COLORS, SRCCOPY
            );
}

internal LRESULT CALLBACK
Win32MainWindowCallback( // "Window Procedure" that lpfnWndProc points to
    HWND Window,    // Handle to the Window
    UINT Message,   // "Window Notification" from Windows OS
    WPARAM WParam,  // Message-dependent data
    LPARAM LParam   // Message-dependent data
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

        case WM_CLOSE:
        {
            // TODO(sustainablelab): Handle this with a message to the user?
            GlobalRunning = false;
        } break;

        case WM_DESTROY:
        {
            // TODO(sustainablelab): Handle this as an error - recreate window?
            GlobalRunning = false;
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        // Catch all keyboard messages. Do not let DefWindowProcA process them.
        case WM_SYSKEYDOWN: // fall through
        case WM_SYSKEYUP:   // fall through
        case WM_KEYDOWN:    // fall through
        case WM_KEYUP:      // All keyboard messages are handled here.
        {
            // Get virtual key code
            uint32 VK_Code = WParam;
            // Get key press information
            bool WasDown = ((LParam & (1<<30)) != 0);
            bool IsDown = ((LParam & (1<<31)) == 0);

            // --- Quit with Alt+F4: I handle quit, not Windows ---
            bool32 AltKeyWasDown = (LParam & (1<<29));
            if ((VK_Code == VK_F4) && AltKeyWasDown)
            {
                GlobalRunning = false;
            }

            // Convert virtual key code to a string
            char *key;
            switch(VK_Code)
            {
                case VK_UP:
                {
                    key = "Up-Arrow";
                } break;
                case VK_DOWN:
                {
                    key = "Down-Arrow";
                } break;
                case VK_LEFT:
                {
                    key = "Left-Arrow";
                } break;
                case VK_RIGHT:
                {
                    key = "Right-Arrow";
                } break;
                case 'W':
                {
                    key = "W";
                } break;
                case 'A':
                {
                    key = "A";
                } break;
                case 'S':
                {
                    key = "S";
                } break;
                case 'D':
                {
                    key = "D";
                } break;
                case 'Q':
                {
                    key = "Q";
                } break;
                case 'E':
                {
                    key = "E";
                } break;
                case VK_SPACE:
                {
                    key = "Space";
                } break;
                case VK_ESCAPE:
                {
                    key = "Escape";
                } break;
                case VK_F4:
                {
                    key = "F4";
                } break;
                case VK_MENU: // Alt
                {
                    key = "Alt";
                } break;
                default:
                {
                    key = "Unused";
                }
            }

            // --- Debug key presses ---
            //
            OutputDebugStringA("Key ");
            OutputDebugStringA(key);
            if (IsDown == WasDown) // held down
            {
                OutputDebugStringA(" HELD down");
            }
            else if (IsDown) // just pressed down now
            {
                OutputDebugStringA(" DOWN");
            }
            else
            {
                OutputDebugStringA(" UP");
            }
            OutputDebugStringA("\n");
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow( DeviceContext,
                                        Dimension.Width, Dimension.Height,
                                        &GlobalBackBuffer
                                      );
            EndPaint(Window, &Paint);
        } break;
        default:
        {
            // Call Window's Default Window Procedure to
            // correctly handle all messages we don't have
            // special instructions for.
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }
    return(Result);
}

internal int CALLBACK
WinMain( // Program entry point
    HINSTANCE Instance,     // Handle to application instance
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int ShowCode
    )
{
    Win32LoadXInput();

    // Create a WindowClass to register for creating a window.
    WNDCLASSA WindowClass = {}; // zero-initialize

    // Allocate memory for the bitmap buffer.
    Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

    WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    // Pointer to func that handles windows notifications
    WindowClass.lpfnWndProc = Win32MainWindowCallback; // <-- fine memory location
    WindowClass.hInstance = Instance; // <--- coarse memory location
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClass(&WindowClass))
    {
        HWND Window = CreateWindowExA(
                0,
                WindowClass.lpszClassName, "Handmade Hero",
                WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                0, 0,
                Instance,
                0
                );
        if (Window) //
        {

            // Note(sustainablelab): Since we specified CS_OWNDC,
            // we can just get one device context and use it
            // forever because we are not sharing it with anyone.
            HDC DeviceContext = GetDC(Window);

            // Initial state of weird gradient: no offset.
            int XOffset = 0;
            int YOffset = 0;

            // --- Setup SOUND here ---
            int32 AudioSeconds = 2;
            int32 AudioChannels = 2; // Stereo
            int32 SamplesPerSecond = 48000; // 48kHz sample rate
            int32 BufferSize = SamplesPerSecond*AudioSeconds*AudioChannels*sizeof(int16); // size of audio buffer in bytes
            Win32InitDSound(Window, SamplesPerSecond, BufferSize);

            // --- GAME LOOP ---
            GlobalRunning = true;
            while(GlobalRunning)
            {
                // Go through the entire queue of messages.
                MSG Message;
                while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if (Message.message == WM_QUIT)
                    {
                        GlobalRunning = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                // TODO(sustainablelab): Should we poll this more frequently?
                for(DWORD ControllerIndex = 0;
                    ControllerIndex < XUSER_MAX_COUNT;
                    ++ControllerIndex)
                {
                    XINPUT_STATE ControllerState;
                    if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                    {
                        // NOTE(sustainablelab): controller is plugged in
                        // TODO(sustainablelab): See if ControllerState.dwPacketNumber increments too rapidly
                        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
                        // https://docs.microsoft.com/en-us/windows/win32/api/xinput/ns-xinput-xinput_gamepad#members
                        bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                        bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                        bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
                        bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
                        bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
                        bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

                        int16 StickX = Pad->sThumbLX;
                        int16 StickY = Pad->sThumbLY;

                        // Interact with weird gradient
                        XOffset += StickX >> 13;
                        YOffset -= StickY >> 13;
                    }
                    else
                    {
                        // NOTE(sustainablelab): controller is not available
                    }
                }

                // Render some image
                RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);

                // Blit
                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow( DeviceContext,
                                            Dimension.Width, Dimension.Height,
                                            &GlobalBackBuffer
                                          );
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
