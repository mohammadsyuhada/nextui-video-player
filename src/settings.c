#include "settings.h"
#include "vp_defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Settings file path (in shared userdata directory)
#define SETTINGS_FILE SHARED_USERDATA_PATH "/video-player/settings.cfg"
#define SETTINGS_DIR SHARED_USERDATA_PATH "/video-player"

void Settings_init(void) {
    // Load settings from file (currently no persistent settings)
}

void Settings_quit(void) {
    Settings_save();
}

void Settings_save(void) {
    // Currently no persistent settings to save
}
