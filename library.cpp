#include "library.h"

#include <iostream>

ABC::ABC()
{

}

ABC::~ABC() {

}

std::vector<double> ABC::GetValues()
{

    std::vector<double> rst;
    rst.push_back(0);
    rst.push_back(1);
    rst.push_back(2);

    return rst;
}

std::string ABC::GetName() {
    return "Name";
}


int MyFunction() { return 999;}
