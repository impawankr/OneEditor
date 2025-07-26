#pragma once

class Terminal {
public:
    Terminal();
    ~Terminal();

    char readKey();   // Read a single character from input
    void enableRawMode();
    void disableRawMode();

private:
    void die(const char* msg);
};

