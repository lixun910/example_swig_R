#ifndef LIBABC_LIBRARY_H
#define LIBABC_LIBRARY_H

#include <vector>

// test wrapping C++ class in R
class ABC {
public:
    ABC();

    virtual ~ABC();

    // test return std::vector in R
    std::vector<double> GetValues();

    // test return std::string in R
    std::string GetName();

    // test public variable
    int numObs;
  
protected:
    int _numKits;
};

// test wrapping function in R
int MyFunction();

#endif
