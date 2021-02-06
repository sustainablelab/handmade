# Casey's debugger

Casey uses flag `/Zi` to get debug symbols.

> `/Zi`
> Produce full-symbolic debugging information in a .pdb file for
> the target using Program Database format.

Casey then uses these symbols in VisualStudio.

# Install gdb

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


