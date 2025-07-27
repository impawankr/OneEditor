#pragma once

#include <iostream>
#include <vector>
#include "../terminal/Terminal.h"
using namespace std;

class Editor {
public:
    Editor();
    void run();
    void openFile(const std::string& filename);

private:
    Terminal terminal;
    int cursorX = 0;
    int cursorY = 0;
    int screenRows = 0;
    int screenCols = 0;
    std::vector<std::string> buffer;
    std::string currentFilename;
    void refreshScreen();
    void processKey(int key);
};

extern Editor editor;  // Global editor instance
