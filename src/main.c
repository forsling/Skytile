#include <stdio.h>
#include "./client.h"
#include "settings.h"

int main(int argc, char* argv[]) {
    if (!load_settings("client.txt", false)) {
        fprintf(stderr, "Failed to load settings\n");
        return 2;
    }

    if (!init_engine()) {
        printf("Failed to initialize engine.\n");
        return 1;
    }

    main_loop();

    cleanup_engine();
    return 0;
}
