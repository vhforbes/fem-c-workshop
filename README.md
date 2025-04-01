# Setup Instructions

Welcome!

For this workshop, you'll need to be running one of these operating systems:
* macOS
* Linux, BSD, or another UNIX variant
* [Linux Subsystem for Windows](https://learn.microsoft.com/en-us/windows/wsl/install) (regular Windows will *not* work for this workshop!)

Any of these operating systems should have everything you need already installed.

To verify this, run the following command in a terminal:

```
gcc -o verify verify.c && ./verify
```

It should print "You're all set!"

## Troubleshooting

If running that command didn't print "You're all set!", you'll need to install either
[GCC](https://gcc.gnu.org/) or [Clang](https://clang.llvm.org/) -
either will work fine, so choose whichever you think will be
easier to install.

These exercises all say to run `gcc`, but you can subsitute `clang` for `gcc` and
it should always work in the case of these examples; `clang` and `gcc` accept
almost identical CLI flags.

Fun fact: macOS actually ships with `clang` but aliases it to `gcc`, so if you run
`gcc --version` on macOS, it prints out `Apple clang version ___`.
