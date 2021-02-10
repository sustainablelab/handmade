The MSYS2 build of gdb works but is pointless because MSYS2 is
not a build environment, so all the entry point stuff is missing
when I start debugging a Windows application.

The minGW build of gdb has broken TUI (text user interface) at
the time of this writing, 2021-02-08.

So just install a pre-built version from GNU -
https://ftp.gnu.org/gnu/gdb/

# Download

Download `gdb-10.1.tar.gz` and `gdb-10.1.tar.gz.sig`.

# Check the signature

`minGW` has `gpg`. Check the signature:

```
$ gpg --verify --keyring ./gnu-keyring.gpg gdb-10.1.tar.gz.sig 
gpg: assuming signed data in 'gdb-10.1.tar.gz'
gpg: Signature made Sat, Oct 24, 2020 12:57:52 AM EDT
gpg:                using DSA key F40ADB902B24264AA42E50BF92EDB04BFF325CF3
gpg: /home/mike/.gnupg/trustdb.gpg: trustdb created
gpg: Good signature from "Joel Brobecker <brobecker@adacore.com>" [unknown]
gpg: WARNING: This key is not certified with a trusted signature!
gpg:          There is no indication that the signature belongs to the owner.
Primary key fingerprint: F40A DB90 2B24 264A A42E  50BF 92ED B04B FF32 5CF3
```

This sounds alarming, but it is the correct output. I have not
added anyone to my web of trust. So `gpg --list-keys` and `gpg
--fingerprint` are both empty.

# Uncompress

`.tar.gz` means I need to uncompress with `gzip` and further
uncompress with `tar`:

```
$ gzip.exe -dc gdb-10.1.tar.gz | tar xf -
```

`-dc` is:

- `-d` -- unzip (as opposed to zip)
- `-c` -- output uncompressed data as a stream

`|` takes that stream and passes it to `tar`

`xf` are flags

- `x` -- extract
- `f` -- file

This takes a few seconds and there is no visible output.

When the command prompt returns, there is a new directory
`gdb-10.1/`.

# Run
God damnit. This is not a pre-built version. It's just source.
It's the same damn source I cloned from Git. Sigh.

git clone git://sourceware.org/git/binutils-gdb.git

So instead of dancing through gpg hoops, I could have just cloned
it again and I'd have the same thing.


