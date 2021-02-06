# PowerShell and MinGW64 mintty terminal

Whereas Casey is cleanly doing everything from one flavor of
environment, the Windows Cmd shell, I'm using a mix of PowerShell
for Windows-specific utilities, and MinGW64 shell, which is
POSIX, for everything else (e.g., running `make`, which Casey
ditches in favor of the simpler `.bat` files).

I continue to use `make`. Casey's `build` to run `build.bat` is
the equivalent of me doing `make -B` to force a rebuild.
