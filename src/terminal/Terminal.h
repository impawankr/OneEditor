#pragma once

class Terminal {
public:
    Terminal();
    ~Terminal();

    int readKey();   // Read a single character from input
    void enableRawMode();
    void disableRawMode();
    
    // get window size declaration
    void getWindowSize(int& rows, int& cols);
private:
    void die(const char* msg);
};

