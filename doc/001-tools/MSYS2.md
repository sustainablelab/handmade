# Use MSYS2

## MSYS2 instead of VisualStudio

I'm using MSYS/MINGW for the VCVRack Plugin tutorial because
that's what's in the tutorial. But I'm also using it because it
is the only option for native Windows builds (that I'm aware of)
without installing VisualStudio as Casey does on Handmade Hero.

## MSYS2 instead of Cygwin

Installing MSYS2 to use the MINGW shell to access the MINGW tools
is ultimately, in theory, no different from starting with a
Cygwin setup, installing MSYS2, and then since any thing on
Windows is callable from Cygwin, simply using the MINGW compilers
instead of the Cygwin versions of the same compilers.

The distinction here is that minGW compilers build native Windows
executables while compilers installed through the Cygwin package
manger build executables with Cygwin DLL dependencies mixed in.

If I'm only interested in building code I write myself, this is
true in practice as well. Nothing wrong with sticking with Cygwin
and specifying the compiler I use either explicitly in my
Makefile or passing the compiler choice at the command line when
I invoke `make`.

But building other people's code, such as VCVRack, means that in
practice it is more convenient to work from the MINGW environment
than the Cygwin environment. My only datapoint for this so far is
`cc`. But I suspect this will be a recurring theme.

The VCVRack `make dep` file invokes `cc` and figures out, from
the name of the resulting compiler, which OS the build is for.

It is common for Makefiles *not* to specify the exact compiler
executable to use.

Of course, if I wanted to build Rack from Cygwin, I could change
the symbolic link within Cygwin.

But `MSYS2/MINGW` has this correct out of the box, so the
toolchain is just simpler to open a MinGW64 shell (mintty
terminal) and then it's basically like I'm using Cygwin, except
of course I don't have any of the Cygwin-installed packages.

The only tool I customize is Vim, and I have that sorted out
where I can easily get my Vim up and running and behaving 99%
percent the way I expect using whatever the latest Vim package is
(and if I need to I'll download the source and build Vim myself).

So I am just going to work from within MSYS2.

MSYS2 also seems the better choice for someone not already
running Cygwin. If Cygwin is not already installed, why have yet
another environment to maintain. For that matter, yet another
reason to go straight to `MSYS2` is to avoid Cygwin's graphical
package manager and just learn `pacman` and do package management
from the command line like everyone else.

