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

{% gist aa6e3b6af11c6fe1351857b85494f5c1 library.h%}
{% gist aa6e3b6af11c6fe1351857b85494f5c1 library.cpp%}
<script src="https://gist.github.com/lixun910/aa6e3b6af11c6fe1351857b85494f5c1.js"></script>

## 1. Building a R Interface using SWIG

Create a SWIG inteface file, which contains the C++ methods/classes you want to access from R:

{% gist aa6e3b6af11c6fe1351857b85494f5c1 library.i%}


> Note: The first line in library.i  `%module libABC` defines the name of the R library. Since R is sensitive to the name of the file and to the file extension in C and C++ mode, the name of the C++ wrapper file must be the name of the library (libABC).

Create R wrapper for your C++ code using SWIG:

```Shell
swig -c++ -o libABC.cpp -r library.i
```

SWIG produces two files for interfacing with R: one is `libABC.R`, which is the wrapper written in R to interface with your C++ code; the other one is `libABC.cpp`, which converts C++ code in `library.cpp` to R callabel routines, and is then compiled with your C++ code as a shared library that `libABC.R` is interfacing with (using .Call function):

```Shell
PKG_LIBS="library.cpp" R CMD SHLIB libABC.cpp
```

The environment variable PKG_LIBS tells R the library.cpp written in C will be part of the compiled shared object library. For example, checking `libABC.cpp`, you will see the R callabel routine of our C++ function `int MyFunction()`:

```C++
SWIGEXPORT SEXP
R_swig_MyFunction ( SEXP s_swig_copy)
{
  int result;
  unsigned int r_nprotect = 0;
  SEXP r_ans = R_NilValue ;
  VMAXTYPE r_vmax = vmaxget() ;
  
  result = (int)MyFunction();
  r_ans = Rf_ScalarInteger(result);
  vmaxset(r_vmax);
  if(r_nprotect)  Rf_unprotect(r_nprotect);
  
  return r_ans;
}
```
> In this file, you can also see that SWIG did many other works for you: e.g. interfacing with STD strings, and vector of integer/double etc.

The R wrapper for this function is in `libABC.R`:
```R
`MyFunction` = function(.copy = FALSE)
{
  ;.Call('R_swig_MyFunction', as.logical(.copy), PACKAGE='libABC');
  
}

attr(`MyFunction`, 'returnType') = 'integer'
class(`MyFunction`) = c("SWIGFunction", class('MyFunction'))
```



> Note: if you read the outputs of the above compiling command, you can see what C++ source files (e.g. libABC.cpp -> libABC.o) are compiles, and what libraries (e.g. libR.dylib) are linked to create the final dynamic library:

```
clang++ -std=gnu++11 -I"/usr/local/Cellar/r/3.6.0_2/lib/R/include" -DNDEBUG   -I/usr/local/opt/gettext/include -I/usr/local/opt/readline/include -I/usr/local/include  -fPIC  -g -O2  -c libABC.cpp -o libABC.o
clang++ -std=gnu++11 -dynamiclib -Wl,-headerpad_max_install_names -undefined dynamic_lookup -single_module -multiply_defined suppress -L/usr/local/opt/gettext/lib -L/usr/local/opt/readline/lib -L/usr/local/lib -L/usr/local/Cellar/r/3.6.0_2/lib/R/lib -L/usr/local/opt/gettext/lib -L/usr/local/opt/readline/lib -L/usr/local/lib -o libABC.so libABC.o library.cpp -L/usr/local/Cellar/r/3.6.0_2/lib/R/lib -lR -lintl -Wl,-framework -Wl,CoreFoundation
```

The output of the above command is a shared library `libABC.so`. Working with `libABC.R`, you can now call your C++ functions/classes in R:

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

## 2. Building a R package

(If you get the above example running successfully) You can package above example as a R library, and distribute it via CRAN or publish it on Github.

The directory structure of a R package is described [here](https://cran.r-project.org/doc/manuals/r-release/R-exts.html#Package-subdirectories). Using our simple example above, we can create a directory structure as following:

```
build/
 |__ libABC/
      |____ demo/
      |____ inst/
      |____ R/
      |     |__ libABC.R
      |____ src/
      |     |__ libABC.cpp
      |     |___ library.cpp
      |     |___ library.h
      |     |___ Makevars
      |
      |____ DESCRIPTION
      |____ NAMESPACE
      |____ LICENSE
```

> NOTE: things are not focused in this post:  the direcotries `demo`, `inst` and  the file `LICENSE`. Please check the document [here](https://cran.r-project.org/doc/manuals/r-release/R-exts.html) for references.

First, we need to copy `libABC.R` file to R/ directory, and copy `libABC.cpp`, `library.h` and `library.cpp` to src/ directory.

To tell R how to compile our C++ source code and wrappers, we also need to create a file `Makevars` under src/ directory. The `Makevars` file is a variant of `GNU Make` that is only for R. Please see document of [R Makevars here](https://cran.r-project.org/doc/manuals/r-release/R-exts.html#Using-Makevars). The most common use of a Makevars file is to set additional preprocessor options (for example include paths and definitions) for C/C++ files via `PKG_CPPFLAGS`, and additional compiler flags by setting `PKG_CFLAGS`, `PKG_CXXFLAGS` or `PKG_FFLAGS`, and linking flag by setting `PKG_LDFLAGS` for C, C++ or Fortran respectively. 


In this example, the Makevars file is empty, since we have library.cpp and libABC.cpp in the src/ directory, and R installer will try to compile them and build a shared library. This is equivlant to running the command above `PKG_LIBS="library.cpp" R CMD SHLIB libABC.cpp` to build the libABC.so library. 

Then, we need to add file `NAMESPACE` to tell R about the namespace associated with our package.


{% gist aa6e3b6af11c6fe1351857b85494f5c1 NAMESPACE %}

The last step is to create a `DESCRPTION` file to tell R about this library when install it from source.

{% gist aa6e3b6af11c6fe1351857b85494f5c1 DESCRIPTIon %}

### Create libABC R package

If you have everything mentioned above in place, you can use the following command to create a R package.

```
cd build
R CMD build libABC
```
The outputs of running the above commands:
```
$cd build

$ R CMD build libABC
* checking for file ‘libABC/DESCRIPTION’ ... OK
* preparing ‘libABC’:
* checking DESCRIPTION meta-information ... OK
* cleaning src
* checking for LF line-endings in source and make files and shell scripts
* checking for empty or unneeded directories
Removed empty directory ‘libABC/demo’
Removed empty directory ‘libABC/inst’
* building ‘libABC_0.0-0.tar.gz’

```


### Use libABC R package

The output file of above command is `libABC_0.0-0.tar.gz`. It can be used as an installer on other machine with R program. To install it from source directly:

```
$ R CMD install libABC_0.0-0.tar.gz 
```

The output of the installation:
```
* installing to library ‘/usr/local/lib/R/3.6/site-library’
* installing *source* package ‘libABC’ ...
** using staged installation
** libs
clang++ -std=gnu++11 -I"/usr/local/Cellar/r/3.6.0_2/lib/R/include" -DNDEBUG   -I/usr/local/opt/gettext/include -I/usr/local/opt/readline/include -I/usr/local/include  -fPIC  -g -O2  -c libABC.cpp -o libABC.o
clang++ -std=gnu++11 -I"/usr/local/Cellar/r/3.6.0_2/lib/R/include" -DNDEBUG   -I/usr/local/opt/gettext/include -I/usr/local/opt/readline/include -I/usr/local/include  -fPIC  -g -O2  -c library.cpp -o library.o
clang++ -std=gnu++11 -dynamiclib -Wl,-headerpad_max_install_names -undefined dynamic_lookup -single_module -multiply_defined suppress -L/usr/local/opt/gettext/lib -L/usr/local/opt/readline/lib -L/usr/local/lib -L/usr/local/Cellar/r/3.6.0_2/lib/R/lib -L/usr/local/opt/gettext/lib -L/usr/local/opt/readline/lib -L/usr/local/lib -o libABC.so libABC.o library.o -L/usr/local/Cellar/r/3.6.0_2/lib/R/lib -lR -lintl -Wl,-framework -Wl,CoreFoundation
installing to /usr/local/lib/R/3.6/site-library/00LOCK-libABC/00new/libABC/libs
** R
** byte-compile and prepare package for lazy loading
** help
No man pages found in package  ‘libABC’ 
*** installing help indices
** building package indices
** testing if installed package can be loaded from temporary location
** checking absolute paths in shared objects and dynamic libraries
** testing if installed package can be loaded from final location
** testing if installed package keeps a record of temporary installation path
* DONE (libABC)
```


If you know the installation locatin of your R progam, you can check the files just being installed:

```
$ cd /usr/local/Cellar/r/3.6.0_2/lib/R/site-library/libABC/
DESCRIPTION  LICENSE      Meta/        NAMESPACE    R/           help/        html/        libs/
```

Now, you can use your libABC R library:

```R
library(libABC)

MyFunction()
# [1] 999

abc<-ABC()
abc$GetValues()
# [1] 0 1 2

abc$GetName()
# [1] "Name"

abc$numObs
# [1] could be any number since we didn't initialize the value of $numObs
```

## Summary

Create a R package with C++ source code using SWIG

1. Create a SWIG interface file (.h) to expose the functions and classes from your C++ source code or library

2. Create R wrappers (.R and .cpp file) using SWIG:

```Shell
swig -c++ -o libABC.cpp -r library.i
```

3. Use `R CMD SHLIB` to build and test the R wrapper

4. If testing is successful, create a R package using the R wrappers with your C++ source code/library.

5. Publish your R package.
