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

# Handmade Hero Day 002

