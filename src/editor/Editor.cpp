#include "Editor.h"
#include <iostream>
#include <unistd.h>

Editor::Editor() {
    // Terminal raw mode is enabled in Terminal constructor
}

Editor::~Editor() {
    // Terminal cleanup is handled by atexit in Terminal
}

void Editor::run() {
    while (true) {
        // --- Clear screen ---
        std::cout << "\x1b[2J";   // ANSI code to clear the screen
        std::cout << "\x1b[H";    // Move cursor to top-left (1;1)

        // --- Draw UI ---
        std::cout << "Simple Editor (C++)\r\n";
        std::cout << "Press 'q' to quit.\r\n";

        // --- Flush output immediately ---
        std::cout.flush();

        // --- Wait for key input ---
        char c = terminal.readKey();

        if (c == 'q') break;

        // Show the key that was pressed
        std::cout << "\r\nYou pressed: '" << c << "' (" << static_cast<int>(c) << ")\r\n";
        std::cout.flush();

        // Sleep briefly to prevent flicker
        usleep(100000);  // 100ms
    }
}

