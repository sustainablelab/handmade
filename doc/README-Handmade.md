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

### paths to includes for Vim `gf`


The source code includes win32 api headers.

*Example:*

```c
#include <windows.h>
```

These win32api headers are here:

```
C:\Program Files (x86)\Windows Kits\10\include\10.0.18362.0\um\Windows.h
```

The source code includes MSVC headers.

*Example:*

```c
#include <stdint.h>
```

These MSVC headers are here:

```
C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.28.29333\include
```

#### Use VisualStudio to find system and compiler headers

- open the source code that includes the header
- right click on the filename in the include statement
    - VisualStudio opens the file in a tab
- right-click the tab name and choose open in folder
- Ctrl+L Ctrl+C to copy the path

#### setup Vim to jump to files

- with cursor on filename, Vim jumps to header files with `gf`
- `gf` requires adding the header path to Vim's `path`

```vim
" HandmadeHero: path to win32api headers
set path=.
set path+=**
let path_win32api='/cygdrive/c/Program\ Files\ (x86)/Windows\ Kits/10/Include/10.0.18362.0/um'
let &path=&path . ',' . path_win32api
let path_msvc='/cygdrive/c/Program\ Files\ (x86)/Microsoft\ Visual\ Studio/2019/Community/VC/Tools/MSVC/14.28.29333/include'
let &path=&path . ',' . path_msvc
```

#### tangent: accidentally found `windows.h`

I originally found `windows.h` by accident. I called a win32
function with the wrong name and got this error
message from the compiler:

```
win32_handmade.cpp|92| warning C4002: too many arguments for function-like macro invocation 'CreateWindowA'
win32_handmade.cpp|78| error C2664: 'HWND CreateWindowExA(DWORD,LPCSTR,LPCSTR,DWORD,int,int,int,int,HWND,HMENU,HINSTANCE,LPVOID)': cannot convert argument 4 from 'const char [14]' to 'DWORD'
win32_handmade.cpp|78| note: There is no context in which this conversion is possible
C:\Program Files (x86)\Windows Kits\10\include\10.0.18362.0\um\winuser.h|4420| note: see declaration of 'CreateWindowExA'
make: *** [Makefile|6| ..\build\win32_handmade.exe] Error 2
```

And I wasn't actually reading the error message. I was using Vim
to jump though the error list. Vim takes my cursor to the
location of the error, so when Vim got to the error on line 78 it
jumped me to line 4420 of the `winuser.h` file.

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

### set up tags files for system and compiler dependencies

`;K` will open a browser tab on DuckDuckGo and put the word under
the cursor in the clipboard for pasting in and searching
docs.microsoft.com

*But!*

*Instead of constantly going to docs.microsoft.com and searching
for stuff...*

I added a `make` recipe to generate a tags file of source
dependencies.

- Vim sets the pats to tags files with the `tags` variable

*Example: look in the pwd for files names `tags` or `lib-tags`*

```vim
set tags=tags,lib-tags
```

- `tags` is the tags file I generate for my source code
    - this is generated/updated with `;cu`
    - `;cu` also generates a cscope database for the more
      powerful cscope probing of the source file
- `lib-tags` is the file I generate for system and compiler
  header files
    - C-file omni-complete uses tags, not cscope, so I make a
      tags file for dependencies
    - this is generated/updated with `make lib-tags`
- simplest thing to do is keep these tags files separate because
  I want to run them separately
    - `;cu` runs real quick and I want to run it all the time on
      my source
    - `make lib-tags` has to do a bunch of stuff that takes a
      little longer:
        - compile `win32_handmade.cpp` with flag `/sourceDependencies lib-tags.json`
            - compile this without linking `/c` because linking is just a
              waste of time since /sourceDependencies gets what it
              needs from the compiler step alone
        - `rm` the `.obj` file that gets generated because I compiled
        - `/sourceDependencies` outputs a JSON file, this is just how
          it is
        - so I wrote a quick JSON parser to grab the info `ctags`
          needs
        - run the JSON parser and create a list of files
        - call `ctags` with `-L` on the list of files

Here is the JSON parser:

```python
# parse-lib-tags-json.py
import json
with open("lib-tags.json") as fin:
    parsed = json.load(fin)
    with open("lib-tags.txt", mode="w") as fout:
        for include in parsed["Data"]["Includes"]:
            fout.write(include)
            fout.write("\n")
```

*Now, whether it's a symbol I define or a symbol defined by the
system header files or the compiler header files, I can tag-jump
and omnicomplete.*

- with cursor on a function name, `<C-]>` jumps to the definition
- typing a function name, Vim omnicomplete with `<C-x><C-o>`
    - this opens a menu of completion options
    - this also opens a preview window with the function name and
      some documentation
- omnicomplete in C/C++ uses the tags file, so `<C-x><C-o>`
  requires generating a `tags` file

The behavior of `;cu` is defined here:

```
/home/mike/.vim/pack/bundle/dev/autoload/ctags.vim
```

`;cu`:

```vim
call system("ctags --c-kinds=+l --exclude=Makefile -R .")
```

- `+l` includes local variables
- `-R .` recurse into the current directory

- `-L file` read from file a list of file names for which tags
  should be generated

To automate the task of making the `lib-tags`, I made this recipe
for a target called `lib-tags`:

```make
.PHONY: lib-tags
lib-tags: win32_handmade.cpp
	cl.exe /c /sourceDependencies lib-tags.json $<
	rm win32_handmade.obj
	python.exe parse-lib-tags-json.py
	rm lib-tags.json
	ctags -f lib-tags --c-kinds=+p -L lib-tags.txt
	rm lib-tags.txt
```

`--c-kinds=+p` includes function prototypes. This is necessary
for CTAGS to pick up the finds of functions Casey is always
looking up. Typical C organization: headers only have signatures,
a.k.a., prototypes, not the function definition, but ctags does
not include these by default. So for `lib-tags` the option to
include prototypes `--c-kinds=+p` is super important, otherwise
the `lib-tags` just has macro definitions and typedefs.

Similarly, if I encounter headers that declare `extern`
variables, I'd want to include those as well: `--c-kinds=+px`.
Or if there's too much "noise" from all the macros, I could
eliminate those from the `lib-tags` with `--c-kinds=+px-d`.

I made the target `PHONY` so that it always runs regardless of
whether file `lib-tags` exists. This made it easier while
creating each of the little tools I used in the recipe. At this
point, there's really no reason for it to be `PHONY`, but I like
having it there as a reminder of what kind of recipe this target
uses.

If `make lib-tags` ever starts taking too long to generate, I can
remove the `PHONY` and then `lib-tags` will only run its recipe
if `win32_handmade.cpp` changed.

### useful ctags tricks to inspect code

*These examples use `-x` which prints human-readable info to
stdout instead of making a tags file.*

List all functions used in `file`:

```bash
ctags  -x  --c-kinds=f  file
```

List all global variables used in file:

```bash
ctags  -x  --c-kinds=v file
```


## back to the code

- add an empty body that returns 0 (OK)
- erase `__clrcall`:


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

## make with MSVC compiler

So I switched to VisualStudio for the debugger. That forced me to
install the compiler tool and the Windows SDK. Something like 6GB
total instead of 1.5GB. OK, fine.

The updated Makefile looks like this:

```make
# https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-by-category?view=msvc-160
CFLAGS = /Zi
IMPORTLIBS = user32.lib

..\build\win32_handmade.exe: win32_handmade.cpp
	cl.exe $(CFLAGS) $(IMPORTLIBS) $< /Fe"$@"
```

That link on top is where I can view all the compiler flags.

To see what the recipe expands to, use `make -n`.

Instead of just `:make`, I made Vim shortcut `;m<Space>` which
invokes `:make -B`. The `-B` flag forces `make` to rebuild
everything. Vim's errorformat works out-of-the-box with the MSVC
compiler (people on the internet say it doesn't and you have to
modify `errorformat` but I didn't have that problem). My shortcut
also opens the quickfix window and puts the cursor on the line
with the first error. So I have all the nice features I'm used to
and I'm still using Casey's style in the ways that matter.

Another reason I'm dropping conditional builds is to get in the
Jonathan Blow's mindset for JAI which I'm looking forward to
adopting when it's available.

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

One kludgey way is to use `DebugView`:

https://docs.microsoft.com/en-us/sysinternals/downloads/debugview#installation-and-use

This is a small utility that listens for debug messages. Run the
utility. Then simply run the executable built with `-g`. Do not
run with the debugger, that suppresses the messages somehow. Just
run the executable by calling it from PowerShell.


# Tips

## do not use `static` (local_persist) in production

but `static` is great for debugging

## do use `static` (global_variable) in production

- init to 0
- keep symbol private to translation unit

## zero-initialize

All don't-care vars are 0.

*Example:*

```c
    WNDCLASSA WindowClass = {}; // zero-initialize
    // Now set the vars I care about.
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    ...
```

## Dealing with XInput.lib

The [Platform
Requirements](https://docs.microsoft.com/en-us/windows/win32/api/xinput/nf-xinput-xinputgetstate#platform-requirements)
to call XInput API functions are sketchy. If we link directly to
`Xinput.lib`:

```make
IMPORTLIBS = user32.lib gdi32.lib xinput.lib
cl.exe $(CFLAGS) $(IMPORTLIBS) $< /Fe"$@"
```

And the player doesn't have the Xinput DLLs installed, then the
game won't load. But the game is playable with a keyboard, so
there's no reason for this.

Casey takes *just* the two functions he needs from `xinput.h` and
does not link to `xinput.lib`! Instead he stubs the functions so
that they do nothing by default and attempts to load the
functions from the `.dll`. In picking which `.dll` to load from,
he does *not* use the `.dll` mentioned in the header or in the
docs because, as he demonstrates, the computer he's on doesn't
have it and it's likely players won't have it either. So he
searches for an earlier version, finds that, assumes that's
universal enough. Here's the command line search:

```cmd
c:\Windows>dir /s xinput1_3.dll
 Volume in drive C has no label.
 Volume Serial Number is 5E8F-2C34

 Directory of c:\Windows\System32

04/04/2007  09:54 PM           107,368 xinput1_3.dll
               1 File(s)        107,368 bytes

 Directory of c:\Windows\SysWOW64

04/04/2007  09:53 PM            81,768 xinput1_3.dll
               1 File(s)         81,768 bytes

     Total Files Listed:
               2 File(s)        189,136 bytes
               0 Dir(s)  92,644,913,152 bytes free
```

See Microsoft examples here:

https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getprocaddress

See how Microsoft locates the `.dll`:

https://docs.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order

Don't need to distribute the .dll but you can. It's called a
"redistributable." But then it has to get installed on the user's
machine, which is not a great idea -- installed lots of crap.
Better to use old .dll that is more likely already there (or is
backwards compatible).

## WeirdGradient movement

```c
// Interact with weird gradient
XOffset += StickX >> 13;
YOffset -= StickY >> 13;
```

Why bit-shift right 13 times? StickX is an `int`. This is a signed
16-bit value. Push full right and StickX == 32767, full left
StickX == -32767. XOffset is also an `int`. So no obvious reason
to bitshift yet.

But XOffset is cast as a byte when it is used. This is how the
pixel color channels are assigned. Casting the int as a byte was
equivalent to doing a mod 255. Since XOffset was simply increment
before, this crude mod 255 was fine.

But now XOffset is not simply incrementing. It's accumulating the
value of StickX. Therefore, doing the mod 255 after adding
XOffset causes the value to alias. The mod 255 is looking at the
8 least significant bits of this add result. Any seeming correct
behavior is an aliasing artifact. The actual behavior should
appear somewhat random.

Bit-shifting by 8 would at least put the results in the right
range where the 8 bits are used are the 8 most significant bits.
But then pushing all the way right or left moves the gradient an
entire box worth, causing way too much of an XOffset. We want
left/right to feel like moving the gradient, so the "strength" of
this needs to be much lower. Bit-shifting 9 bits moves half-box
at a time, still too much. 10 bits a quarter-box, 11 bits an
eighth-box, 12 bits a sixteenth-box. Finally, 13 bits moves the
background a maximum of a thirty-second-box at a time, which
feels about right.

## Keyboard input

Windows has key down/up messages and system/non-system key
messages. Combining the possibilities, there are four messages:

```c
WM_SYSKEYDOWN: // F10-key pressed or Alt+key pressed
WM_SYSKEYUP:   // F10-key released or Alt+key released
WM_KEYDOWN:    // other key pressed
WM_KEYUP:      // other key released
```

If any of these cases are left out of the `switch` block, the
`DefWindowProcA` will handle the case. This removes the message
from the queue and I am unable to process it myself.

I claim the message for myself by providing a case.

Rather than code for each case, Casey just uses empty cases to
catch them all, then puts all of his code in the keyup case. This
does not mean Casey's response is only to keyup messages. The
keyup case is catching *all* of the key-related messages.

This struck me as odd and I tried doing it the other way: handle
key-down things in the key-down case and key-up things in the
key-up case. But having `Alt` pressed down prevented my from
registering when `F4` was released. I'd like to revisit this
later, but for now I'm keeping it simple: do what Casey does and
set up the cases as a fall-through so that all key-handling
happens in one case statement.

```c
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
            bool32 AltKeyWasDown = ((LParam & (1<<29)) != 0);
            if ((VK_Code == VK_F4) && AltKeyWasDown)
            {
                GlobalRunning = false;
            }

            // ---EXPLANATION---

            // LParam bit 29 is the "Context Code":
            //  - 1 if ALT key was down when message was generated
            //  - 0 otherwise
            // Bit 29 is ALWAYS 0 in WM_KEYDOWN messages (see docs).
            // Bit 29 is ALWAYS 0 in WM_KEYUP messages (see docs).
            // Ignore those docs. I tested this in the debugger!
            //
            // I see "Alt DOWN". Then I press "F4".
            // IsDown is True and WasDown is False.
            // That means pressing "F4" is a "WM_KEYDOWN" message!
            //
            // AltKeyWasDown is True.
            // This sets GlobalRunning to False.
            // So set a breakpoint on that line to do the
            // following test.
            //
            // The game loop has to iterate again before exiting,
            // so the case statement will finish executing my
            // key-press debug print code.
            //
            // The debug print out goes like this:
            // Hold down Alt:
            // "Alt DOWN" <------------- debug output
            // Press F4 and the code breaks. Let go of Alt and F4.
            // Step through the code with F11:
            // "F4" <------------------- debug output
            // Keep stepping
            // "F4 DOWN" <-------------- debug output
            // (Keep stepping and it eventually quits.)
```

The only thing I'm doing differently is I went ahead and put a
switch block to read the virtual key codes (`VK_Code`) instead of
the if-else placeholders. I did this *now* because I wanted to
play around with key presses and see messages in the debugger and
I quickly realized I could write that code in *one* place if I
used the switch case to grab the `VK_Code` instead of
copying-and-pasting the debug print messages it into every
if-else statement.

Here's the flavor of Casey's if-else placeholder version. The
nice thing about this is it's simple to read and to quickly try
out something based on a particular key.

```c
        // Casey's temporary KEYUP code with if statements
        case WM_KEYUP: // key released
        {
            uint32 VK_Code = WParam;
            bool WasDown = ((LParam & (1<<30)) != 0);
            bool IsDown = ((LParam & (1<<31)) == 0);
            OutputDebugStringA("Key RELEASED: ");
            if (VK_Code == VK_UP)
            {
                OutputDebugStringA("Up-Arrow\n");
            }
            if (VK_Code == VK_DOWN)
            {
                OutputDebugStringA("Down-Arrow\n");
            }
            if (VK_Code == VK_LEFT)
            {
                OutputDebugStringA("Left-Arrow\n");
            }
            if (VK_Code == VK_RIGHT)
            {
                OutputDebugStringA("Right-Arrow\n");
            }
            if (VK_Code == 'W')
            {
                OutputDebugStringA("W\n");
            }
            if (VK_Code == 'A')
            {
                OutputDebugStringA("A\n");
            }
            if (VK_Code == 'S')
            {
                OutputDebugStringA("S\n");
            }
            if (VK_Code == 'D')
            {
                OutputDebugStringA("D\n");
            }
            if (VK_Code == 'Q')
            {
                OutputDebugStringA("Q\n");
            }
            if (VK_Code == 'E')
            {
                OutputDebugStringA("E\n");
            }
            if (VK_Code == VK_SPACE)
            {
                OutputDebugStringA("Space\n");
            }
            if (VK_Code == VK_ESCAPE)
            {
                OutputDebugStringA("Escape\n");
            }
        } break;
```

# Sound

## DirectSound .dll
Find `dsound.dll`:

```cmd
c:\Windows>dir /s dsound.dll
 Volume in drive C has no label.
 Volume Serial Number is 5E8F-2C34

 Directory of c:\Windows\System32

12/07/2019  04:08 AM           615,424 dsound.dll
               1 File(s)        615,424 bytes

 Directory of c:\Windows\SysWOW64

12/07/2019  04:09 AM           493,056 dsound.dll
               1 File(s)        493,056 bytes

 Directory of c:\Windows\WinSxS\amd64_microsoft-windows-audio-dsound_31bf3856ad364e35_10.0.19041.1_none_0e8ccbdbe140657b

12/07/2019  04:08 AM           615,424 dsound.dll
               1 File(s)        615,424 bytes

 Directory of c:\Windows\WinSxS\wow64_microsoft-windows-audio-dsound_31bf3856ad364e35_10.0.19041.1_none_18e1762e15a12776

12/07/2019  04:09 AM           493,056 dsound.dll
               1 File(s)        493,056 bytes

     Total Files Listed:
               4 File(s)      2,216,960 bytes
               0 Dir(s)  92,395,655,168 bytes free
```

## Concept

Sound is stored in a buffer in memory. Sound access works as a
looping write-head/read-head over that memory. Each sample of
stereo audio is stored 16bits of LEFT 16 bits of RIGHT. So there
are 4 bytes per sample of audio. The buffer size is this 4-byte
sample size times the sampling rate times the seconds of audio
length of this buffer.

## Windows DirectSound uses COM trash

DirectSound uses the Component Object Model (COM) disaster.
This API sucks. The docs suck.

Instead of a simple function call API, its all object based.
The unnecessarily complicated setup goes like this:

- create an "interface" with DirectSoundCreate()
- then call "interface" methods
- the first method you must call is to SetCooperativeLevel()
- then create the primary and secondary buffers
- these both take structs for buffer descriptions and wave
  (audio) formats, so it's a lot of setting up structs to make
  calls and the parameters for the struct values are poorly
  documented

