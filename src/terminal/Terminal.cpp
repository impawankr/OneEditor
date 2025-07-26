#include "Terminal.h"
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>

static struct termios orig_termios;  // Stores original terminal settings

// Helper to exit with an error
void Terminal::die(const char* msg) {
    perror(msg);
    exit(1);
}

// Restore terminal to original state
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// Enable raw mode
void Terminal::enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");

    atexit(::disableRawMode);  // Register restore handler

    struct termios raw = orig_termios;

    // Input flags: disable break, CR to NL, parity check, strip, IXON
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // Output flags: disable post-processing
    raw.c_oflag &= ~(OPOST);

    // Control flags: 8-bit chars
    raw.c_cflag |= (CS8);

    // Local flags: disable echoing, canonical mode, signals, extended input
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

    // Set minimum input to 0, timeout to 1 (100ms)
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

// Constructor
Terminal::Terminal() {
    enableRawMode();
}

// Destructor — no need to disable manually; handled by atexit()
Terminal::~Terminal() {}

// Read a single key (non-blocking, short timeout)
char Terminal::readKey() {
    char c;
    ssize_t n;

    while ((n = read(STDIN_FILENO, &c, 1)) != 1) {
        if (n == -1 && errno != EAGAIN)
            die("read");
    }

    return c;
}

