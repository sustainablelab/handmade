# subst

Casey says this is an Amiga cultural thing. Cool.

Use `subst` from Powershell.

```
PS C:\Users\mike> subst /?
Associates a path with a drive letter.

SUBST [drive1: [drive2:]path]
SUBST drive1: /D

  drive1:        Specifies a virtual drive to which you want to assign a path.
  [drive2:]path  Specifies a physical drive and path you want to assign to
                 a virtual drive.
  /D             Deletes a substituted (virtual) drive.

Type SUBST with no parameters to display a list of current virtual drives.
```

The `subst` command works in the MinGW64 shell, but the help
command `subst /?` does not:

```
$ subst /?
Invalid parameter - C:/msys64/?
```

`subst` has no `-h` or `--help` flag, it's from a different
command-line argument passing culture.

So, from Powershell, I'm going to name my folder `work` just like
Casey, and assign it to drive `w`:

```
subst w: C:\msys64\home\mike\work\
```

And check the new drive name exists:

```
PS C:\Users\mike> subst
W:\: => C:\msys64\home\mike\work\
```

Now I can `cd` into `w`:

```
PS C:\Users\mike> cd w:
PS w:\>
```

Similarly, `MinGW` knows about `w`:

```
$ subst
W:\: => C:\msys64\home\mike\work\
```

Notice how the symbol after `MINGW64` changes after I `cd` into
`w`. The `~` means home. The `/w` means the `W` drive.

```
mike@DESKTOP-H26981V MINGW64 ~
$ cd w:

mike@DESKTOP-H26981V MINGW64 /w
$
```

To make this `subst` mapping to `w` drive permanent, add this to
the PowerShell profile. I open PowerShell to launch my MinGW64
shell, so this is a fine place to run this command.

I detailed how to set this file up here:

https://sustainablelab.github.io/powershell/

This is the path to my profile:

```
/cygdrive/c/Users/Mike/Documents/WindowsPowerShell/Microsoft.PowerShell_profile.ps1
```


