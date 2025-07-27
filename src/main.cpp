#include "editor/Editor.h"

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        editor.openFile(argv[1]);
    }

    editor.run();  // start the editor
    return 0;
}

