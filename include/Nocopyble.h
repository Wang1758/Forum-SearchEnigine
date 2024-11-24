#pragma once
#include <iostream>

class Nocopyble {
protected:
    Nocopyble() {}
    ~Nocopyble() {}
    Nocopyble(const Nocopyble&) = delete ;
    Nocopyble& operator=(const Nocopyble&) = delete;
};