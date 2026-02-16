#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "vp_defines.h"
#include "api.h"
#include "config.h"
#include "ui_fonts.h"

#define APP_FONT_PATH "res/font.ttf"
#define FONT_TITLE_SIZE 28
#define FONT_XLARGE_SIZE 36

typedef struct {
    TTF_Font* xlarge;
    TTF_Font* title;
    TTF_Font* large;
    TTF_Font* medium;
    TTF_Font* small;
    TTF_Font* tiny;
} AppFonts;

static AppFonts app_font = {0};

void Fonts_load(void) {
    app_font.xlarge = TTF_OpenFont(APP_FONT_PATH, SCALE1(FONT_XLARGE_SIZE));
    app_font.title = TTF_OpenFont(APP_FONT_PATH, SCALE1(FONT_TITLE_SIZE));
    app_font.large = TTF_OpenFont(APP_FONT_PATH, SCALE1(FONT_LARGE));
    app_font.medium = TTF_OpenFont(APP_FONT_PATH, SCALE1(FONT_MEDIUM));
    app_font.small = TTF_OpenFont(APP_FONT_PATH, SCALE1(FONT_SMALL));
    app_font.tiny = TTF_OpenFont(APP_FONT_PATH, SCALE1(FONT_TINY));
}

void Fonts_unload(void) {
    if (app_font.xlarge) { TTF_CloseFont(app_font.xlarge); app_font.xlarge = NULL; }
    if (app_font.title) { TTF_CloseFont(app_font.title); app_font.title = NULL; }
    if (app_font.large) { TTF_CloseFont(app_font.large); app_font.large = NULL; }
    if (app_font.medium) { TTF_CloseFont(app_font.medium); app_font.medium = NULL; }
    if (app_font.small) { TTF_CloseFont(app_font.small); app_font.small = NULL; }
    if (app_font.tiny) { TTF_CloseFont(app_font.tiny); app_font.tiny = NULL; }
}

TTF_Font* Fonts_getXLarge(void) { return app_font.xlarge; }
TTF_Font* Fonts_getTitle(void) { return app_font.title; }
TTF_Font* Fonts_getArtist(void) { return app_font.medium; }
TTF_Font* Fonts_getAlbum(void) { return app_font.small; }
TTF_Font* Fonts_getLarge(void) { return app_font.large; }
TTF_Font* Fonts_getMedium(void) { return app_font.medium; }
TTF_Font* Fonts_getSmall(void) { return app_font.small; }
TTF_Font* Fonts_getTiny(void) { return app_font.tiny; }

SDL_Color Fonts_getListTextColor(bool selected) {
    return selected ? uintToColour(THEME_COLOR5_255) : uintToColour(THEME_COLOR4_255);
}

void Fonts_drawListItemBg(SDL_Surface* screen, SDL_Rect* rect, bool selected) {
    if (selected) {
        GFX_blitPillColor(ASSET_WHITE_PILL, screen, rect, THEME_COLOR1, RGB_WHITE);
    }
}

int Fonts_calcListPillWidth(TTF_Font* font, const char* text, char* truncated, int max_width, int prefix_width) {
    int available_width = max_width - prefix_width;
    int padding = SCALE1(BUTTON_PADDING * 2);
    int raw_text_w, raw_text_h;
    TTF_SizeUTF8(font, text, &raw_text_w, &raw_text_h);
    if (raw_text_w + padding > available_width) {
        GFX_truncateText(font, text, truncated, available_width, padding);
        return max_width;
    }
    strncpy(truncated, text, 255);
    truncated[255] = '\0';
    return MIN(max_width, prefix_width + raw_text_w + padding);
}
