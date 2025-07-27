#include "Editor.h"
#include "../input/KeyCodes.h"
#include <unistd.h>
#include <vector>
#include <cstring>
#include <fstream>

Editor::Editor() {
    buffer = {
        "Welcome to MyTextEdit!",
        "Use arrow keys to move the cursor.",
        "This is editable dummy text.",
        "Press 'q' to quit."
    };
}

void Editor::refreshScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);      // Clear screen
    write(STDOUT_FILENO, "\x1b[H", 3);       // Move to top-left

    terminal.getWindowSize(screenRows, screenCols);  // Get terminal size

    // Draw text buffer
    for (int i = 0; i < screenRows - 1; ++i) {  // Leave last row for status
        if (i < buffer.size()) {
            write(STDOUT_FILENO, buffer[i].c_str(), buffer[i].size());
        }
        write(STDOUT_FILENO, "\r\n", 2);
    }

    // Draw status bar
    char status[80];
    snprintf(status, sizeof(status), "EDITOR -- Pos: (%d, %d)", cursorY + 1, cursorX + 1);

    std::string statusBar = "\x1b[7m";  // Start inverted
    statusBar += status;

    // Pad with spaces to reach screen width
    while (statusBar.size() < screenCols) {
    statusBar += ' ';
    }
    statusBar += "\x1b[m";  // Reset formatting

    write(STDOUT_FILENO, statusBar.c_str(), statusBar.size());


    // Move cursor to actual position
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", cursorY + 1, cursorX + 1);
    write(STDOUT_FILENO, buf, strlen(buf));
}


void Editor::processKey(int key) {
    switch (key) {
        case 'q':
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);

        case ARROW_UP:
            if (cursorY > 0) cursorY--;
            break;
        case ARROW_DOWN:
            if (cursorY < (int)buffer.size() - 1) cursorY++;
            break;
        case ARROW_LEFT:
            if (cursorX > 0) cursorX--;
            break;
        case ARROW_RIGHT:
            if (cursorY < (int)buffer.size()) {
                if (cursorX < (int)buffer[cursorY].size()) cursorX++;
            }
            break;
        case BACKSPACE:
            if (cursorX > 0){
                buffer[cursorY].erase(cursorX-1, 1);
                cursorX--;
            }
            else if (cursorY > 0) {
                // Merge with previous line
                cursorX = buffer[cursorY - 1].length();
                buffer[cursorY - 1] += buffer[cursorY];
                buffer.erase(buffer.begin() + cursorY);
                cursorY--;
            }
            break;
        case '\r':
            // Handle Enter key
            if (cursorY >= buffer.size()) {
                buffer.push_back("");
            }
            buffer.insert(buffer.begin() + cursorY + 1, buffer[cursorY].substr(cursorX));
            buffer[cursorY] = buffer[cursorY].substr(0, cursorX);
            cursorY++;
            cursorX = 0;
            break;
        case SAVEFILE:
            // Handle custom save key (Ctrl+S)
            {
                std::ofstream outFile("output.txt");
                for (const auto& line : buffer) {
                    outFile << line << "\n";
                }
                outFile.close();
                write(STDOUT_FILENO, "\x1b[2J", 4);
                write(STDOUT_FILENO, "\x1b[H", 3);
                write(STDOUT_FILENO, "File saved to output.txt\n", 26);
            }
            break;
    }

    // 🆕 Insert printable characters
    if (key >= 32 && key <= 126) {
        if (cursorY >= buffer.size()) {
            buffer.push_back("");
        }

        buffer[cursorY].insert(cursorX, 1, (char)key);
        cursorX++;
    }

    // Clamp cursorX if it's beyond current line length
    if (cursorY < buffer.size()) {
        if (cursorX > (int)buffer[cursorY].length()) {
            cursorX = buffer[cursorY].length();
        }
    }
}


void Editor::run() {
    while (true) {
        refreshScreen();
        int key = terminal.readKey();
        processKey(key);
    }
}

