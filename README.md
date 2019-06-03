# Create a R package with C++ source code using SWIG

by Xun Li

Note: There are many tutorials and examples about how to use Rcpp to write a R library (and create a package), which will not be covered by this post. 

This post will discuss how to use SWIG, which is a software to connect your C++ program with other programming languages, to wrap your C++ source code to a R library.

All following notes are based on using Mac OSX 10.14.

- SWIG (version 4.0): brew install SWIG
- R (version 3.6.0): brew install R
- Xcode (also need to [install command line tools](http://osxdaily.com/2014/02/12/install-command-line-tools-mac-os-x/))
- CLANG (Apple LLVM version 9.1.0, comes from Xcode 9 command line tools)


Your C++ code:

{% gist aa6e3b6af11c6fe1351857b85494f5c1 %}

## Building a R Interface using SWIG

Create a SWIG inteface file, which contains the C++ methods/classes you want to access from R:

// file above: library.i

> Note: The first line in library.i  `%module libABC` defines the name of the R library. Since R is sensitive to the name of the file and to the file extension in C and C++ mode, the name of the C++ wrapper file must be the name of the library (libABC).

Create R wrapper for your  C++ code:

```Shell
swig -c++ -o libABC.cpp -r library.i
```

SWIG produces two files for interfacing with R: one is `libABC.R`, which is the wrapper written in R to interface with your C++ code; the other one is `libABC.cpp`, which is then compiled with your C++ code as a shared library that `libABC.R` is interfacing with:

```Shell
PKG_LIBS="library.cpp" R CMD SHLIB libABC.cpp
```

> Note: if you read the outputs of the above compiling command, you can see what C++ source files (e.g. libABC.cpp -> libABC.o) are compiles, and what libraries (e.g. libR.dylib) are linked to create the final dynamic library:

```
clang++ -std=gnu++11 -I"/usr/local/Cellar/r/3.6.0_2/lib/R/include" -DNDEBUG   -I/usr/local/opt/gettext/include -I/usr/local/opt/readline/include -I/usr/local/include  -fPIC  -g -O2  -c libABC.cpp -o libABC.o
clang++ -std=gnu++11 -dynamiclib -Wl,-headerpad_max_install_names -undefined dynamic_lookup -single_module -multiply_defined suppress -L/usr/local/opt/gettext/lib -L/usr/local/opt/readline/lib -L/usr/local/lib -L/usr/local/Cellar/r/3.6.0_2/lib/R/lib -L/usr/local/opt/gettext/lib -L/usr/local/opt/readline/lib -L/usr/local/lib -o libABC.so libABC.o library.cpp -L/usr/local/Cellar/r/3.6.0_2/lib/R/lib -lR -lintl -Wl,-framework -Wl,CoreFoundation
```

The output of the above command is a shared library `libABC.so`. Working with `libABC.R`, you can call your C++ functions/classes in R:

```R
dyn.load(paste("libABC", .Platform$dynlib.ext, sep=""))
source("libABC.R")
cacheMetaData(1)

# call function from libABC
MyFunction()
# [1] 999

# create instance ABC from libABC
abc <- ABC()
abc
# An object of class "_p_ABC"
# Slot "ref":
# <pointer: 0x7f8bc9d1ef40>

# test public member functions of ABC
abc$GetValues()
# [1] 0 1 2

abc$GetName()
# [1] "Name"

# test public member attributes of ABC
abc$numObs
# [1] 690504514 since we didn't initialize the value of $numObs
```

> Note: the cacheMetaData(1) will cause R to refresh its object tables. Without it, inheritance of wrapped objects may fail.

