#include "Editor.h"
#include "../input/KeyCodes.h"
#include <unistd.h>
#include <vector>
#include <cstring>
#include <fstream>

Editor editor;  // Global editor instance

Editor::Editor() {
    buffer = {
        "Welcome to MyTextEdit!",
        "Use arrow keys to move the cursor.",
        "This is editable dummy text.",
        "Press 'q' to quit."
    };
    
    // Initialize cursor and offset positions
    cursorX = 0;
    cursorY = 0;
    rowOffset = 0;
    colOffset = 0;
    
    // Get initial terminal size
    terminal.getWindowSize(screenRows, screenCols);
    if (screenRows <= 0) screenRows = 24;
    if (screenCols <= 0) screenCols = 80;
}

// Scroll function
void Editor::scroll() {
    // Get actual terminal size
    terminal.getWindowSize(screenRows, screenCols);
    
    // Ensure we have at least some screen space
    if (screenRows <= 0) screenRows = 24;
    if (screenCols <= 0) screenCols = 80;
    
    // Adjust rowOffset based on cursor position
    if (cursorY == 0) {
        // Special case: if cursor is at the very top, ensure first line is visible
        rowOffset = 0;
    } else if (cursorY < rowOffset) {
        rowOffset = cursorY;
    } else if (cursorY >= rowOffset + screenRows - 2) {  // Leave 1 row for status bar
        rowOffset = cursorY - (screenRows - 3);  // Adjust for status bar
    }
    
    // Ensure rowOffset doesn't go negative
    if (rowOffset < 0) rowOffset = 0;

    // Adjust colOffset based on cursor position
    if (cursorX < colOffset) {
        colOffset = cursorX;
    } else if (cursorX >= colOffset + screenCols) {
        colOffset = cursorX - screenCols + 1;
    }
    
    // Ensure colOffset doesn't go negative
    if (colOffset < 0) colOffset = 0;
}

void Editor::refreshScreen() {
    scroll();
    write(STDOUT_FILENO, "\x1b[2J", 4); // Clear screen
    write(STDOUT_FILENO, "\x1b[H", 3);  // Move cursor to top-left

    int lineNumberWidth = showLineNumbers ? 6 : 0;
    int displayRows = screenRows - 1;  // Leave 1 row for status bar

    for (int i = 0; i < displayRows; ++i) {
        int fileRow = rowOffset + i;
        std::string displayLine;

        if (fileRow < static_cast<int>(buffer.size())) {
            if (showLineNumbers) {
                char lineNum[16];
                snprintf(lineNum, sizeof(lineNum), "%4d | ", fileRow + 1);
                displayLine += lineNum;
            }

            std::string line = buffer[fileRow];
            if (static_cast<int>(line.length()) > colOffset)
                line = line.substr(colOffset, screenCols - lineNumberWidth);
            else
                line = "";

            displayLine += line;
        }

        write(STDOUT_FILENO, displayLine.c_str(), displayLine.length());
        write(STDOUT_FILENO, "\r\n", 2);
    }

    // Draw status bar
    char status[80];
    snprintf(status, sizeof(status), "EDITOR -- Pos: (%d, %d) | RowOffset: %d", cursorY + 1, cursorX + 1, rowOffset);

    std::string statusBar = "\x1b[7m";  // Start inverted
    statusBar += status;

    while (statusBar.size() < static_cast<size_t>(screenCols)) {
        statusBar += ' ';
    }
    statusBar += "\x1b[m";  // End inverted
    write(STDOUT_FILENO, statusBar.c_str(), statusBar.length());
    write(STDOUT_FILENO, "\r\n", 2);

    // Position the cursor - ensure it's visible
    int cursorRow = (cursorY - rowOffset) + 1;
    int cursorCol = (cursorX - colOffset) + (showLineNumbers ? lineNumberWidth + 1 : 1);
    
    // Ensure cursor position is within visible area
    if (cursorRow < 1) cursorRow = 1;
    if (cursorRow > displayRows) cursorRow = displayRows;
    if (cursorCol < 1) cursorCol = 1;
    if (cursorCol > screenCols) cursorCol = screenCols;
    
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", cursorRow, cursorCol);
    write(STDOUT_FILENO, buf, strlen(buf));
}


void Editor::processKey(int key) {
    switch (key) {
        case 'q':
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            saveFile(currentFilename);
            exit(0);
        case SAVEFILE:
            saveFile(currentFilename);
            break;
        case ARROW_UP:
            if (cursorY > 0) cursorY--;
            break;
        case ARROW_DOWN:
            if (cursorY < static_cast<int>(buffer.size()) - 1) cursorY++;
            break;
        case ARROW_LEFT:
            if (cursorX > 0) cursorX--;
            break;
        case ARROW_RIGHT:
            if (cursorY < (int)buffer.size()) {
                if (cursorX < (int)buffer[cursorY].size()) cursorX++;
            }
            break;
        case PAGE_UP:
            cursorY -= screenRows - 1;
            if (cursorY < 0) cursorY = 0;
            break;
        case PAGE_DOWN:
            cursorY += screenRows - 1;
            if (cursorY >= static_cast<int>(buffer.size())) cursorY = buffer.size() - 1;
            break;
        case HOME:
            cursorX = 0;
            break;
        case END:
            if (cursorY < static_cast<int>(buffer.size())) {
                cursorX = buffer[cursorY].length();
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
        case LINENUMBER:
                showLineNumbers = !showLineNumbers;
                break;
        case '\r':
            // Handle Enter key
            if (cursorY >= static_cast<int>(buffer.size())) {
                buffer.push_back("");
            }
            buffer.insert(buffer.begin() + cursorY + 1, buffer[cursorY].substr(cursorX));
            buffer[cursorY] = buffer[cursorY].substr(0, cursorX);
            cursorY++;
            cursorX = 0;
            break;
    }

    // 🆕 Insert printable characters
    if (key >= 32 && key <= 126) {
        if (cursorY >= static_cast<int>(buffer.size())) {
            buffer.push_back("");
        }

        buffer[cursorY].insert(cursorX, 1, (char)key);
        cursorX++;
    }

    // Clamp cursorX if it's beyond current line length
    if (cursorY < static_cast<int>(buffer.size())) {
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

void Editor::openFile(const std::string& filename) {
    currentFilename = filename;
    std::ifstream inFile(filename);
    if (!inFile.is_open()) return;

    std::string line;
    buffer.clear();

    while (std::getline(inFile, line)) {
        buffer.push_back(line);
    }

    inFile.close();
    cursorX = 0;
    cursorY = 0;
}

void Editor::saveFile(const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) return;

    for (const auto& line : buffer) {
        outFile << line << "\n";
    }

    outFile.close();
}

