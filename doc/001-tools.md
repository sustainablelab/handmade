# TLDR
Install VisualStudio Community. Unclick all the garbage, just
get:

- the core VisualStudio (~1.5GB)
- the MSVC compiler tools (~1.5GB)
- the latest Win10SDK (~3GB)

Update PowerShell Profile with a `vcvars` command to run the
`vcvarsall.bat` script that sets the environment variables. This
makes `cl` a recognized command in PowerShell, and `cl.exe` a
recognized command from Cygwin.

# A long path
- MSYS2
- MinGW64
    - always build from here, never from MSYS2
    - compiler is `cc.exe`, `cc` is not a symbolic link to gcc or
      clang, don't know what it is
- subst -- get a `w:` drive
- lldb plus DebugView
    - debugger to replace VisualStudio
    - not sure it's better than gdb
    - both are text based (until I can get `gdb -tui` to work
      under minGW)
- DebugView -- view OutputDebugString messages
    - other Windows messages show up here too, so it's a little
      noisy
    - the other messages are from Oculus, and there are a lot of
      them!
    - but I can definitely see the OutputDebugString messages
- I gave in and installed VisualStudio for the debugger
- I also gave in and installed MSVC, another 1.5GB
    - I wanted to step the debugger
    - but it cannot step through the code because it doesn't know
      where any of the C runtime stuff is in MinGW and I don't
      know either
- Now that I am using the VisualStudio debugger and MSVC
  compiler, I can go back to using Cygwin -- there is no reason
  to work from within a MinGW shell, I'm not using any of its
  tools anymore.
- Somehow MinGW has `<windows.h>`, but in VisualStudio it's
  another 3GB module to install...
