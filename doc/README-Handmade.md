# Handmade Hero Day 001

- Set up the tools.
- Set up a project.

## Project setup

Make a handmade folder:

```
cd w:
mkdir handmade/code
cd handmade/code
```

Create a barebones source file for compiling.

```
vim win32_handmade.cpp
```

Look up `WinMain` on the interwebs. This will take you to
`docs.microsoft` (formerly `MSDN`):

https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-winmain

Click the `Copy` link and paste the code into the `.cpp` file:


```c
/* ===========
 * $File: $
 * $Date: $
 * $Revision: $
 * ===========
 */

int __clrcall WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nShowCmd
);
```

Fix:

- change the indentation to four spaces
- include `windows.h`
- add an empty body that returns 0 (OK)
- erase `__clrcall`:

```c
#include <windows.h>

int WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd
    )
{
    return 0;
}
```

Casey makes a build script:

```
vim build.bat
```

```build.bat
@echo off
```

The trouble is I cannot run this from my POSIX enivronment. I can
run it from PowerShell. But not sure how to trigger this from
Vim, so I'm going to just use a Makefile. I'm sure there's a way
to call `cmd.exe` or something like that, but it's not worth
doing all this because I'm not using VisualStudio and exactly
matching Casey's toolchain.

So change `build.bat` to `Makefile`.

```
vim Makefile
```

```make
.PHONY: print-vars
print-vars:
	@echo bob
```

Run `make` and you get `bob`:

```
$ make
bob
```

Identify the compiler.

Replace this with a real Makefile:


```make
CFLAGS = 

../build/out.exe: win32_handmade.cpp
	cc $(CFLAGS) $< -o $@
```

I have details to explain this here:

https://github.com/sustainablelab/example-c-project/blob/master/doc/Tutorial.md#3---build-with-a-makefile

Now from `~/work/handmade/code` I can just call `make`:

```
make
```

And it puts `out.exe` in the `build` folder.

I'm still not sure exactly what tool `cc` is. It's whatever the
MinGW64 environment provides as `cc.exe`:

```
$ whereis cc
cc: /mingw64/bin/cc.exe
```

And whatever `cc.exe` is, it seems capable of building from
C++ source.

So I hit `make` to run the first target in my `Makefile`, and
Casey hits `build` to run his `build.bat` file. Casey's `build`
is the equivalent of `make` with the `-B` flag, which forces
everything to rebuild:

```
$ make -B
cc  win32_handmade.cpp -o ../build/out.exe
```

Versus without the `-B` flag:

```
$ make
make: '../build/out.exe' is up to date.
```

The -B flag forces the rebuild even though `build/out.exe` is
already up-to-date. Casey claims the builds will never take more
than a few seconds. So in a way, forcing a rebuild every time
forces him to be more disciplined as an engineer. I am looking
forward to that!


## Debug
Use `gdb` instead of VisualStudio.

## C Runtime Library

Casey needs to link to the C runtime library because of the
toolchain he's using.

https://docs.microsoft.com/en-us/cpp/c-runtime-library/c-run-time-library-reference?view=msvc-160

I don't think I need to link to it.

## Windows libraries

The "hello world" opens a message box using the `MessageBox`
function. The documentation is here:

https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox

And scrolling to the bottom, we see this is part of `user32.lib`.
In this instance, I did not need to add an explicit linker flag
to this `lib` or the `user32.dll`.

## Symbols

`-g` to output debugging symbols. This is to inspect values in
memory with the debugger.

# Handmade Hero Day 002

Yesterday I built `out.exe` that popped up a dialog box. Today I
replace that with opening a window.

## Make a struct of window info

I define a struct variable that holds the window info.

```c
// Important.
// The struct is all the values that need to get passed to some
// function.

// Not important.
// C++ automatically does the `typdef struct` for you.
// Microsoft is doing the typedef struct explicitly for
// compatibility. The `tagWNDCLASSEXA` is unnecessary, but
// whatever, that's how Microsoft wrote it.
/* typedef struct tagWNDCLASSEXA { */
/*   UINT      cbSize; */
/*   UINT      style; */
/*   WNDPROC   lpfnWndProc; */
/*   int       cbClsExtra; */
/*   int       cbWndExtra; */
/*   HINSTANCE hInstance; */
/*   HICON     hIcon; */
/*   HCURSOR   hCursor; */
/*   HBRUSH    hbrBackground; */
/*   LPCSTR    lpszMenuName; */
/*   LPCSTR    lpszClassName; */
/*   HICON     hIconSm; */
/* } WNDCLASSEXA, *PWNDCLASSEXA, *NPWNDCLASSEXA, *LPWNDCLASSEXA; */
```

The variable has a lot of members. Instead of setting them all to
zero, I set the variable equal to `{}`.

```c
{
    WNDCLASSEXA WindowClass = {}; // initialize all to 0
    return 0;
}
```

## Characterize how this syntax is handled

Will this *zero-initialization* shorthand have the intended
effect? Easier to check in the debugger than try to dig up
specifics for this exact C++ compiler and hardware target.

I compile with flag `-g` for debug symbols and use `lldb` to
inspect the value of the struct members before and after the
*zero-initialization*.

Run `lldb`:

```
lldb
```

Pick a file to debug:

```
file ../build/out.exe
(lldb) target create "../build/out.exe"
Current executable set to 'C:\msys64\home\mike\work\handmade\build\out.exe' (x86_64).
```

I don't know how to use `watchpoint`, so I'm going to halt
execution and view all the variables instead.

Halt execution upon entering main:

```
breakpoint set --name WinMain
(lldb) breakpoint set --name WinMain
Breakpoint 1: where = out.exe`::WinMain(HINSTANCE, HINSTANCE, LPSTR, int) + 24 at win32_handmade.cpp:17:13, address = 0x0000000140001558
```

Run until the breakpoint:

```
r
(lldb) r
Process 63816 launched: 'C:\msys64\home\mike\work\handmade\build\out.exe' (x86_64)
Process 63816 stopped
* thread #1, stop reason = breakpoint 1.1
    frame #0: 0x00007ff6d3451558 out.exe`::WinMain(hInstance=0x00007ff6d3450000, hPrevInstance=0x0000000000000000, lpCmdLine="", nShowCmd=10) at win32_handmade.cpp:17:13
   14       int       nShowCmd
   15       )
   16   {
-> 17   WNDCLASSEXA WindowClass = {}; // initialize all to 0
                    ^
   18       return 0;
   19   }
   20  
```

View the variable values:

```
frame variable
(lldb) frame variable
(HINSTANCE) hInstance = 0x00007ff6d3450000
(HINSTANCE) hPrevInstance = 0x0000000000000000
(LPSTR) lpCmdLine = 0x000001808e4a4992 ""
(int) nShowCmd = 10
(WNDCLASSEXA) WindowClass = {
  cbSize = 3544520128
  style = 32758
  lpfnWndProc = 0x0000000000000010
  cbClsExtra = 0
  cbWndExtra = 0
  hInstance = 0x0000000000000000
  hIcon = 0x000001808e3b46f0
  hCursor = 0x00007ff6d3452749
  hbrBackground = 0x0000000000000001
  lpszMenuName = 0x0000000000000030 ""
  lpszClassName = 0x000001808e3b46b0 "0G;\x8e\x80\U00000001"
  hIconSm = 0x0000000000000010
}
```

Step the code:

```
n
(lldb) n
Process 63816 stopped
* thread #1, stop reason = step over
    frame #0: 0x00007ff6d34515a8 out.exe`::WinMain(hInstance=0x00007ff6d3450000, hPrevInstance=0x0000000000000000, lpCmdLine="", nShowCmd=10) at win32_handmade.cpp:18:12
   15       )
   16   {
   17   WNDCLASSEXA WindowClass = {}; // initialize all to 0
-> 18       return 0;
                   ^
   19   }
   20  
   21   // Important.
```

View the variables again:

```
frame variable
(lldb) frame variable
(HINSTANCE) hInstance = 0x00007ff6d3450000
(HINSTANCE) hPrevInstance = 0x0000000000000000
(LPSTR) lpCmdLine = 0x000001808e4a4992 ""
(int) nShowCmd = 10
(WNDCLASSEXA) WindowClass = {
  cbSize = 0
  style = 0
  lpfnWndProc = 0x0000000000000000
  cbClsExtra = 0
  cbWndExtra = 0
  hInstance = 0x0000000000000000
  hIcon = 0x0000000000000000
  hCursor = 0x0000000000000000
  hbrBackground = 0x0000000000000000
  lpszMenuName = 0x0000000000000000
  lpszClassName = 0x0000000000000000
  hIconSm = 0x0000000000000000
}
```

Yes, this line of code sets all members of struct `WindowClass` to zero:

```
WNDCLASSEXA WindowClass = {};
```

Also, using the `frame variable` command in `lldb` let me see the
names of the struct variables. I get this from the `doc.microsoft`
page, but it's nice to have it in `lldb` because then it's just
another Vim buffer I can easily copy from.

## Configure the window

## Print to OutputDebugString

https://docs.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-outputdebugstringa

A good way to bootstrap things. Have no way to print yet or put
things on screen. This lets us see the message in the debugger.

Trouble is, I'm looking for a way to see these messages without
installing VisualStudio.

