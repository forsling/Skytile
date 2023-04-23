#include <stdio.h>
#include "./engine.h"
#include "settings.h"

int main(int argc, char* argv[]) {
    if (!init_engine()) {
        printf("Failed to initialize engine.\n");
        return 1;
    }

    if (!load_settings("game_settings.txt")) {
        fprintf(stderr, "Failed to load settings\n");
        return 1;
    }

    main_loop();

    cleanup_engine();
    return 0;
}
