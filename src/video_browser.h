#ifndef __VIDEO_BROWSER_H__
#define __VIDEO_BROWSER_H__

#include <stdbool.h>
#include "vp_defines.h"

typedef struct {
    char name[256];
    char path[512];
    bool is_dir;
    VideoFormat format;
} VideoFileEntry;

typedef struct {
    char current_path[512];
    VideoFileEntry* entries;
    int entry_count;
    int selected;
    int scroll_offset;
    int items_per_page;
} VideoBrowserContext;

// Detect video format from filename extension
VideoFormat VideoBrowser_detectFormat(const char* filename);

// Check if file is a supported video format
bool VideoBrowser_isVideoFile(const char* filename);

// Free browser entries
void VideoBrowser_freeEntries(VideoBrowserContext* ctx);

// Load directory contents (video files + directories)
void VideoBrowser_loadDirectory(VideoBrowserContext* ctx, const char* path, const char* root);

// Get display name for file (without extension)
void VideoBrowser_getDisplayName(const char* filename, char* out, int max_len);

// Count video files in browser
int VideoBrowser_countVideoFiles(const VideoBrowserContext* ctx);

// Check if browser has parent entry (..)
bool VideoBrowser_hasParent(const VideoBrowserContext* ctx);

// Find subtitle file matching video (returns true if found, fills sub_path)
bool VideoBrowser_findSubtitle(const char* video_path, char* sub_path, int sub_path_size);

#endif
