#include <stdio.h>
#include <string.h>

#include "vp_defines.h"
#include "api.h"
#include "module_common.h"
#include "module_settings.h"
#include "ui_main.h"
#include "ui_settings.h"
#include "selfupdate.h"
#include "wifi.h"

// Internal states
typedef enum {
    SETTINGS_STATE_ABOUT,
    SETTINGS_STATE_UPDATING
} SettingsState;

// Internal app state constants for controls help
#define SETTINGS_INTERNAL_ABOUT 41

ModuleExitReason SettingsModule_run(SDL_Surface* screen) {
    SettingsState state = SETTINGS_STATE_ABOUT;
    int dirty = 1;
    int show_setting = 0;

    while (1) {
        PAD_poll();

        // Handle global input first
        GlobalInputResult global = ModuleCommon_handleGlobalInput(screen, &show_setting, SETTINGS_INTERNAL_ABOUT);
        if (global.should_quit) {
            return MODULE_EXIT_QUIT;
        }
        if (global.input_consumed) {
            if (global.dirty) dirty = 1;
            GFX_sync();
            continue;
        }

        // State-specific handling
        switch (state) {
            case SETTINGS_STATE_ABOUT:
                SelfUpdate_update();
                {
                    const SelfUpdateStatus* status = SelfUpdate_getStatus();

                    // Keep refreshing while checking for updates
                    if (status->state == SELFUPDATE_STATE_CHECKING) {
                        dirty = 1;
                    }

                    if (PAD_justPressed(BTN_A)) {
                        if (status->update_available) {
                            SelfUpdate_startUpdate();
                            state = SETTINGS_STATE_UPDATING;
                            dirty = 1;
                        } else if (status->state != SELFUPDATE_STATE_CHECKING) {
                            if (Wifi_ensureConnected(screen, show_setting)) {
                                SelfUpdate_checkForUpdate();
                            }
                            dirty = 1;
                        }
                    }
                    else if (PAD_justPressed(BTN_B)) {
                        return MODULE_EXIT_TO_MENU;
                    }
                }
                break;

            case SETTINGS_STATE_UPDATING:
                // Disable autosleep during update
                ModuleCommon_setAutosleepDisabled(true);

                SelfUpdate_update();
                {
                    const SelfUpdateStatus* update_status = SelfUpdate_getStatus();
                    SelfUpdateState update_state = update_status->state;

                    if (update_state == SELFUPDATE_STATE_COMPLETED) {
                        if (PAD_justPressed(BTN_A)) {
                            // Quit to apply update
                            ModuleCommon_setAutosleepDisabled(false);
                            return MODULE_EXIT_QUIT;
                        }
                    }
                    else if (PAD_justPressed(BTN_B)) {
                        if (update_state == SELFUPDATE_STATE_DOWNLOADING) {
                            SelfUpdate_cancelUpdate();
                        }
                        ModuleCommon_setAutosleepDisabled(false);
                        state = SETTINGS_STATE_ABOUT;
                        dirty = 1;
                    }
                }

                // Always redraw during update
                dirty = 1;
                break;
        }

        // Handle power management
        ModuleCommon_PWR_update(&dirty, &show_setting);

        // Render
        if (dirty) {
            switch (state) {
                case SETTINGS_STATE_ABOUT:
                    render_about(screen, show_setting);
                    break;
                case SETTINGS_STATE_UPDATING:
                    render_app_updating(screen, show_setting);
                    break;
            }

            if (show_setting) {
                GFX_blitHardwareHints(screen, show_setting);
            }

            GFX_flip(screen);
            dirty = 0;
        } else {
            GFX_sync();
        }
    }
}
