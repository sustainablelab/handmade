# https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-by-category?view=msvc-160
CFLAGS = /Zi
IMPORTLIBS = user32.lib

..\build\win32_handmade.exe: win32_handmade.cpp
	cl.exe $(CFLAGS) $(IMPORTLIBS) $< /Fe"$@"
