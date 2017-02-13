Installation
------------

The package is forked from tanger, so 

1) Get and install LLVM. You need the llvm-gcc binaries and compile LLVM
   from source.

2) Configure and build Taglibc:
   ./configure --with-llvmsrc=/your/path/to/llvm/sourcedir \
               --with-llvmobj=/your/path/to/llvm/builddir
   ./build

Don't use `make' to build, it will miss most of the directories.

There is a test application in directory `test'.

