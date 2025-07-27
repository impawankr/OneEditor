#include "Terminal.h"
#include <termios.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include "../input/KeyCodes.h"
#include <fcntl.h> // fcntl is library for file control operations
#include <sys/ioctl.h> // ioctl is library for terminal I/O control
#include <errno.h> // errno is used for error handling 
                   
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

void Terminal::getWindowSize(int& rows, int& cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        rows = 0;
        cols = 0;
    } else {
        rows = ws.ws_row;
        cols = ws.ws_col;
    }
}

// Constructor
Terminal::Terminal() {
    enableRawMode();
}

// Destructor — no need to disable manually; handled by atexit()
Terminal::~Terminal() {}

// Read a single key (non-blocking, short timeout)
int Terminal::readKey() {
    char c;
    ssize_t n;

    while ((n = read(STDIN_FILENO, &c, 1)) != 1) {
        if (n == -1 && errno != EAGAIN)
            die("read");
    }

    if (c == '\x1b') {
        char seq[3];

        // Try to read the next 2 bytes of the escape sequence
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return ARROW_UP;
                case 'B': return ARROW_DOWN;
                case 'C': return ARROW_RIGHT;
                case 'D': return ARROW_LEFT;
                case '5': {
                    char tilde;
                    if (read(STDIN_FILENO, &tilde, 1) == 1 && tilde == '~')
                        return PAGE_UP;
                    break;
                }
                case '6': {
                    char tilde;
                    if (read(STDIN_FILENO, &tilde, 1) == 1 && tilde == '~')
                        return PAGE_DOWN;
                    break;
                }
                case 'H': return HOME;
                case 'F': return END;
            }
        }

        return '\x1b';  // Unknown sequence
    }

    return c;  // Normal key
}
