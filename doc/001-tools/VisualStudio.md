# Setup

Install VisualStudio for the debugger

- 2019 Community
- unclick everything except the 1.5GB of core stuff and the 1.5GB
  of MSVC tools
    - the MSVC tools are necessary to use the compiler
    - even though I can use the `cc.exe` compiler from MinGW, and
      run the debugger in VisualStuio, I cannot use the `F11`
      "debug stepping" feature because VisualStudio cannot find
      the C runtime source files

Now, to *use* the compiler in MSVC, you have to first run
vcvarsall.bat. Casey does this by adding \k
w:\handmade\misc\shell.bat to his cmd.exe shortcut. And in
shell.bat he has this:

```
@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
```

I do something similar, but using PowerShell.

## load vcvarsall.bat env vars into PowerShell

In my PowerShell Profile, I add the following:

```
function LoadVisualStudioEnvVars {
    cmd.exe /K "`"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat`" & powershell.exe"
}
Set-Alias -Name vcvars -Value LoadVisualStudioEnvVars
```

The main idea is launching cmd and running the .bat file:

```
cmd.exe /K ".\vcvars64.bat"
```

The `/K` lets me pass an argument to `cmd.exe`. The `.bat` file
sets environment variables. PowerShell cannot run the `.bat` file
on its own because the environment variables are lost after the
`.bat` file is done running.

The `& powershell.exe` opens a new PowerShell in this PowerShell
(so beware, the PowerShell profile runs again!) and lets
the new PowerShell inherit the environment variables:

```
cmd.exe /K "`".\vcvars64.bat`" & powershell.exe"
```

The down-side to doing this is now the prompt is three levels
deep:

- original PowerShell
- cmd
- new PowerShell

So you have to "exit" three times to close the shell.

The back-quote is how you escape quotes in PowerShell. And the
quotes are necessary when the path contains spaces.

# Use

Open VisualStudio to debug the build:

```
cd work/handmade/build/
devenv out.exe &
```

In VisualStudio:

- F5 starts debug, focus switches to launched application
- switch back to VisualStudio, Shift+F5 stops debug
- F11 steps through the code, but this instantly runs into
  `crtexe.c not found` because I'm using mingw cc instead of MSVC

When you close VisualStudio after launching it this way, it will
ask to save the solution it made for running out.exe. Do save
this. Put it in the build folder.

Always do this the first time running a new executable. After
that, you can just launch VisualStudio and let it use the
"solution" it created. Read "solution" as "session."

To launch just VisualStudio:

```
mike@DESKTOP-H26981V MINGW64 ~/work/handmade
$ devenv &
[1] 1845
```

