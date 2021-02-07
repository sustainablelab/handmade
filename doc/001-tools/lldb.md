# Casey's debugger

Casey uses flag `/Zi` to get debug symbols.

> `/Zi`
> Produce full-symbolic debugging information in a .pdb file for
> the target using Program Database format.

Casey then uses these symbols in VisualStudio with the
`Watchlist`.

# My first attempt with `gdb`

I use compiler flag `-g` to get debug symbols. I then use these
symbols in `gdb` with `watch` to set a watchpoint. A watchpoint
stops execution whenever the value changes. The value is the
value of the variable that I watch. This way I can look at a
variable's value every time the code changes its value.

OK, so the build of gdb for minGW does not have the TUI enabled,
so you cannot "watch" anything because there is no TUI.

I installed `gdb` for `msys2` (the fact it does not come
installed is telling, similar story to `cc` not being linked to
anything).

The build of gdb for msys2 does have TUI enabled, but it seems to
be missing other things.

# My switch to `lldb`
Before fixing the latter or rebuilding the former, I just
searched for packages with the word `debug`:

```
pacman -Ss debug
```

# Install lldb

And I found `LLDB`. Install it:

```
pacman -S mingw-w64-x86_64-lldb
```

Here is the documentation:

https://lldb.llvm.org/use/tutorial.html#

# Run lldb


## Example

```
$ lldb out.exe
(lldb) target create "out.exe"
Current executable set to 'C:\msys64\home\mike\work\handmade\build\out.exe' (x86_64).
breakpoint set --name WinMain
(lldb) breakpoint set --name WinMain
Breakpoint 1: where = out.exe`::WinMain(HINSTANCE, HINSTANCE, LPSTR, int) + 24 at win32_handmade.cpp:17:13, address = 0x0000000140001558
r
(lldb) r
Process 27748 launched: 'C:\msys64\home\mike\work\handmade\build\out.exe' (x86_64)
Process 27748 stopped
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
(lldb) c
(lldb) c
Process 27748 resuming
Process 27748 exited with status = 0 (0x00000000)
(lldb) exit
(lldb) exit
```

An application window opens and the code stops at line 17. Press
`n` to execute the next line of code. The WindowClass variable is
created and the arrow points at line 18. Now `n` again to execute
`return 0`. Unexpectedly, `n` literally steps line-by-line
through the source code. So pressing `n` again puts me on the
closing curly braces of WinMain()! Finally, subsequent pressing
`n` I get the message `Process 28788 stopped`:

```
(lldb) n
Process 28788 stopped
* thread #1, stop reason = step over
    frame #0: 0x00007ff6d34513c1 out.exe`__tmainCRTStartup at crtexe.c:323:8
(lldb) n
(lldb) n
Process 28788 stopped
* thread #1, stop reason = step over
    frame #0: 0x00007ff6d34513c7 out.exe`__tmainCRTStartup at crtexe.c:321:13
```

## help breakpoint set

I copied the useful looking arguments below. The full `help` is
much longer.

```
help breakpoint set
(lldb) help breakpoint set
Sets a breakpoint or set of breakpoints in the executable.

Syntax: breakpoint set <cmd-options>

Command Options Usage:
  breakpoint set [-DHd] -l <linenum> ...
    ...
       -f <filename> ( --file <filename> )
            Specifies the source file in which to set this breakpoint.  Note,
            by default lldb only looks for files that are #included if they use
            the standard include file extensions.  To set breakpoints on
            .c/.cpp/.m/.mm files that are #included, set
            target.inline-breakpoint-strategy to always.
   ...
       -l <linenum> ( --line <linenum> )
            Specifies the line number on which to set this breakpoint.
    ...
       -n <function-name> ( --name <function-name> )
            Set the breakpoint by function name.  Can be repeated multiple
            times to makeone breakpoint for multiple names

       -o <boolean> ( --one-shot <boolean> )
            The breakpoint is deleted the first time it stop causes a stop.
    ...
       -r <regular-expression> ( --func-regex <regular-expression> )
            Set the breakpoint by function name, evaluating a
            regular-expression to find the function name(s).
    ...
```

## command history
The up/down arrows do not walk through history. But `command
history` works like the Linux `history` tool:

```
command history
(lldb) command history
   0: file ../build/out.exe
   1: help breakpoint set
   2: l
   3: source list
   4: breakpoint set --name WinMain
   5: breakpoint
   6: breakpoint list
   7: n
   8: r
   9: n
  10: help
  11: apropos history
  12: command history
```

Re-run a command like this: `!6`

```
!6
(lldb) !6
Current breakpoints:
1: name = 'WinMain', locations = 1, resolved = 1, hit count = 1
  1.1: where = out.exe`::WinMain(HINSTANCE, HINSTANCE, LPSTR, int) + 24 at win32_handmade.cpp:17:13, address = 0x00007ff6d3451558, resolved, hit count = 1 
```

## apropos

`apropos history` is how I found `command history`. Use `apropos`
to search for functionality. For example, I want a watchlist:

```
apropos watch
(lldb) apropos watch
The following commands may relate to 'watch':
  watchpoint                -- Commands for operating on watchpoints.
  watchpoint command        -- Commands for adding, removing and examining LLDB
                               commands executed when the watchpoint is hit
                               (watchpoint 'commands').
  watchpoint command add    -- Add a set of LLDB commands to a watchpoint, to
                               be executed whenever the watchpoint is hit.
  watchpoint command delete -- Delete the set of commands from a watchpoint.
  watchpoint command list   -- List the script or set of commands to be
                               executed when the watchpoint is hit.
  watchpoint delete         -- Delete the specified watchpoint(s).  If no
                               watchpoints are specified, delete them all.
  watchpoint disable        -- Disable the specified watchpoint(s) without
                               removing it/them.  If no watchpoints are
                               specified, disable them all.
  watchpoint enable         -- Enable the specified disabled watchpoint(s). If
                               no watchpoints are specified, enable all of them.
  watchpoint ignore         -- Set ignore count on the specified watchpoint(s).
                               If no watchpoints are specified, set them all.
  watchpoint list           -- List all watchpoints at configurable levels of
                               detail.
  watchpoint modify         -- Modify the options on a watchpoint or set of
                               watchpoints in the executable.  If no watchpoint
                               is specified, act on the last created
                               watchpoint.  Passing an empty argument clears
                               the modification.
  watchpoint set            -- Commands for setting a watchpoint.
  watchpoint set expression -- Set a watchpoint on an address by supplying an
                               expression. Use the '-w' option to specify the
                               type of watchpoint and the '-s' option to
                               specify the byte size to watch for. If no '-w'
                               option is specified, it defaults to write. If no
                               '-s' option is specified, it defaults to the
                               target's pointer byte size. Note that there are
                               limited hardware resources for watchpoints. If
                               watchpoint setting fails, consider
                               disable/delete existing ones to free up
                               resources.
  watchpoint set variable   -- Set a watchpoint on a variable. Use the '-w'
                               option to specify the type of watchpoint and the
                               '-s' option to specify the byte size to watch
                               for. If no '-w' option is specified, it defaults
                               to write. If no '-s' option is specified, it
                               defaults to the variable's byte size. Note that
                               there are limited hardware resources for
                               watchpoints. If watchpoint setting fails,
                               consider disable/delete existing ones to free up
                               resources.
```

Well, even with that, I couldn't figure out how to use watchlist.

## frame

```
frame info
(lldb) frame info
frame #0: 0x00007ff6d3451558 out.exe`::WinMain(hInstance=0x00007ff6d3450000, hPrevInstance=0x0000000000000000, lpCmdLine="", nShowCmd=10) at win32_handmade.cpp:17:13
```

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

```
frame info
(lldb) frame info
frame #0: 0x00007ff6d34515a8 out.exe`::WinMain(hInstance=0x00007ff6d3450000, hPrevInstance=0x0000000000000000, lpCmdLine="", nShowCmd=10) at win32_handmade.cpp:18:12
```

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

# Install gdb

Trying other gdb packages... Hoping to get `-tui`.

```
$ pacman -S $(pacman -Ssq x86_64-gdb-multiarch)
```

No luck.

Cloned `binutils-gdb` repo to `/home/mike`:

```
git clone git://sourceware.org/git/binutils-gdb.git
```

Create and enter a build folder:

```
mkdir ~/dev-gdb
cd ~/dev-gdb
```

Run the top-level `configure`:

```
$ ../binutils-gdb/configure --prefix=/home/mike/dev-gdb --with-curses
configure: loading site script /mingw64/etc/config.site
checking build system type... x86_64-w64-mingw32
checking host system type... x86_64-w64-mingw32
checking target system type... x86_64-w64-mingw32
checking for a BSD-compatible install... /usr/bin/install -c
...
```

The `--prefix` is usually `/usr/bin`, but I don't want to clutter
that folder with an experimental build.

The `--with-curses` is the whole reason I'm building from source.
This should give me the `-tui` option.

## Old attempot

The MSYS2 debugger package is `gdb`.

The general purpose mingw version of `gdb` is already installed:

```
$ pacman -Ss gdb
...
mingw64/mingw-w64-x86_64-gdb 10.1-2 (mingw-w64-x86_64-toolchain) [installed]
    GNU Debugger (mingw-w64)
...
msys/gdbm 1.19-1 (Database) [installed]
    GNU database library
msys/libgdbm 1.19-1 (libraries) [installed]
    GNU database library
```

Hidden by my `...`, there are also `gdb` versions for AVR and
ARM. Check these out as a supplement to my current method of
debugging firmware, which is to read the disassembly.
Unfortunately, I suspect these are intended to run on the actual
target, so I'd need a JTAG link to actually use them. I don't
think they are meant to run as emulated hardware on my
development machine.

# Run gdb

First, see that `gdb` is running the `mingw` `gdb`:

```
$ cd handmade/build
$ gdb out.exe
GNU gdb (GDB) 10.1
Copyright (C) 2020 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "x86_64-w64-mingw32".           <----------------- YES
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from out.exe...
(gdb) q
```

To see the call stack like Casey does, first start `gdb` in quiet
mode:

```
$ gdb -q ../build/out.exe 
Reading symbols from ../build/out.exe...
```

Now set a breakpoint at `WinMain`:

```
(gdb) b WinMain
Breakpoint 1 at 0x140001544
```

Run to the breakpoint:

```
(gdb) r  
Starting program: W:\handmade\build\out.exe 
[New Thread 69204.0x10fd4]
[New Thread 69204.0x10bfc]
[New Thread 69204.0x11110]

Thread 1 hit Breakpoint 1, 0x00007ff793571544 in WinMain ()
```

Find where we are in the code:

```
(gdb) list
3       in C:/_/M/mingw-w64-crt-git/src/mingw-w64/mingw-w64-crt/crt/crt0_c.c
```

## crt0.c

That is interesting. I'm guessing `crt` is C Runtime Library. So
we didn't link or anything to the C Runtime Library, but here we
are in `crt0_c.c`. I don't even know where this file is. From
Cygwin, so as not to disturb `gdb`, I use `find`:

```
$ cd /cygdrive/c/msys64
$ find . -name 'crt'
./mingw64/lib/terminfo/63/crt
./mingw64/share/licenses/crt
./mingw64/share/terminfo/63/crt
```

But that's a different `crt` -- those are for some sort of VT220
emulator. OK, giving up on this now. Don't know what source code
we're sitting in, but whatever.

Back to the debugging...

Show the call stack with `backtrace`:

```
(gdb) bt 
#0  0x00007ff793571544 in WinMain ()
#1  0x00007ff7935713c1 in __tmainCRTStartup ()
    at C:/_/M/mingw-w64-crt-git/src/mingw-w64/mingw-w64-crt/crt/crtexe.c:321
#2  0x00007ff7935714f6 in mainCRTStartup ()
    at C:/_/M/mingw-w64-crt-git/src/mingw-w64/mingw-w64-crt/crt/crtexe.c:202
(gdb) 
```

Casey sees `kernel32.dll` and `ntdll.dll` instead of my
`crtexe.c`. But that's OK. The function name is the same one he
sees.

- `__tmainCRTStartup()`
- `mainCRTStartup()`

Casey is able to open the source for this from VisualStudio. I'm
not. That's fine. This really doesn't matter. Read all about
`crt0.c` here:

https://en.wikipedia.org/wiki/Crt0

# Build with debug symbols

```make
CFLAGS = -g
```

