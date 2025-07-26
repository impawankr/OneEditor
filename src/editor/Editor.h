#pragma once

#include "../terminal/Terminal.h"

class Editor {
public:
    Editor();
    ~Editor();

    void run();

private:
    Terminal terminal;
};

