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
    int screenRows = 24; //temporary default
    int screenCols = 80; //temporary default
    std::vector<std::string> buffer;
    std::string currentFilename;
    void saveFile(const std::string& filename);
    void refreshScreen();
    void processKey(int key);
    //scroll function declaration
    void scroll();

    // Scrolling perameters
    int rowOffset = 0;
    int colOffset = 0;
};

extern Editor editor;  // Global editor instance
