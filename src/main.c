#include <stdio.h>
#include "./engine.h"

int main(int argc, char* argv[]) {
    if (!init_engine()) {
        printf("Failed to initialize engine.\n");
        return 1;
    }

    main_loop();

    cleanup_engine();
    return 0;
}
