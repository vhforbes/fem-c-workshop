# Setup Instructions

Welcome!

For this workshop, you'll need to be running one of these operating systems:
* macOS
* Linux, BSD, or another UNIX variant
* [Linux Subsystem for Windows](https://learn.microsoft.com/en-us/windows/wsl/install) (regular Windows will *not* work for this workshop!)

Any of these operating systems should have everything you need already installed.

To verify this, run the following command in a terminal:

```
cc -o verify verify.c && ./verify
```

It should print "You're all set!"

## Troubleshooting

If running that command didn't print "You're all set!", you'll need to install either
[GCC](https://gcc.gnu.org/) or [Clang](https://clang.llvm.org/) -
either will work fine, so choose whichever you think will be
easier to install. If installing one of those two does not provide
the above `cc` command, then whenever you see instructions to
run `cc` in this workshop, you can replace the `cc` command
with either `gcc` or `clang` (depending on which you installed).
