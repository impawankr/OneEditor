#pragma once

// Any value > 255 avoids clashing with normal characters
enum EditorKey {
    SAVEFILE = 19, // Custom key for saving file
    BACKSPACE = 127, // ASCII backspace
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    PAGE_UP,
    PAGE_DOWN,
    HOME,
    END
};

