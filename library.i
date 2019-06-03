%module libABC

%include "std_string.i"
%include "std_vector.i"

// copied code in bracket verbatim into the wrapper code 
%{
    #include "library.h"
%}

%include "library.h"
