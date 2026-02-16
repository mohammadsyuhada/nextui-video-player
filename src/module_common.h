#ifndef __MODULE_COMMON_H__
#define __MODULE_COMMON_H__

#include <SDL2/SDL.h>
#include <stdbool.h>

#define TOAST_DURATION 3000

typedef enum {
    MODULE_EXIT_TO_MENU,
    MODULE_EXIT_QUIT
} ModuleExitReason;

typedef struct {
    bool input_consumed;
    bool should_quit;
    bool dirty;
} GlobalInputResult;

void ModuleCommon_init(void);
GlobalInputResult ModuleCommon_handleGlobalInput(SDL_Surface* screen, int* show_setting, int app_state);
void ModuleCommon_setAutosleepDisabled(bool disabled);
void ModuleCommon_tickToast(char* message, uint32_t toast_time, int* dirty);
void ModuleCommon_quit(void);
void ModuleCommon_PWR_update(int* dirty, int* show_setting);

#endif
