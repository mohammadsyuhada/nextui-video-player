#ifndef __UI_FONTS_H__
#define __UI_FONTS_H__

#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

void Fonts_load(void);
void Fonts_unload(void);
TTF_Font* Fonts_getXLarge(void);
TTF_Font* Fonts_getTitle(void);
TTF_Font* Fonts_getArtist(void);
TTF_Font* Fonts_getAlbum(void);
TTF_Font* Fonts_getLarge(void);
TTF_Font* Fonts_getMedium(void);
TTF_Font* Fonts_getSmall(void);
TTF_Font* Fonts_getTiny(void);
SDL_Color Fonts_getListTextColor(bool selected);
void Fonts_drawListItemBg(SDL_Surface* screen, SDL_Rect* rect, bool selected);
int Fonts_calcListPillWidth(TTF_Font* font, const char* text, char* truncated, int max_width, int prefix_width);

#endif
