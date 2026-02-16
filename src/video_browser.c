#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#include "vp_defines.h"
#include "api.h"
#include "video_browser.h"

// Case-insensitive extension check helper
static int ext_match(const char* ext, const char* target) {
    return strcasecmp(ext, target) == 0;
}

// Detect video format from filename extension
VideoFormat VideoBrowser_detectFormat(const char* filename) {
    if (!filename) return VIDEO_FORMAT_UNKNOWN;

    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return VIDEO_FORMAT_UNKNOWN;

    const char* ext = dot + 1;

    if (ext_match(ext, VIDEO_EXT_MP4))  return VIDEO_FORMAT_MP4;
    if (ext_match(ext, VIDEO_EXT_MKV))  return VIDEO_FORMAT_MKV;
    if (ext_match(ext, VIDEO_EXT_AVI))  return VIDEO_FORMAT_AVI;
    if (ext_match(ext, VIDEO_EXT_WEBM)) return VIDEO_FORMAT_WEBM;
    if (ext_match(ext, VIDEO_EXT_MOV))  return VIDEO_FORMAT_MOV;
    if (ext_match(ext, VIDEO_EXT_TS))   return VIDEO_FORMAT_TS;
    if (ext_match(ext, VIDEO_EXT_FLV))  return VIDEO_FORMAT_FLV;
    if (ext_match(ext, VIDEO_EXT_M4V))  return VIDEO_FORMAT_M4V;
    if (ext_match(ext, VIDEO_EXT_WMV))  return VIDEO_FORMAT_WMV;
    if (ext_match(ext, VIDEO_EXT_MPG))  return VIDEO_FORMAT_MPG;
    if (ext_match(ext, VIDEO_EXT_MPEG)) return VIDEO_FORMAT_MPG;
    if (ext_match(ext, VIDEO_EXT_3GP))  return VIDEO_FORMAT_3GP;

    return VIDEO_FORMAT_UNKNOWN;
}

// Check if file is a supported video format
bool VideoBrowser_isVideoFile(const char* filename) {
    return VideoBrowser_detectFormat(filename) != VIDEO_FORMAT_UNKNOWN;
}

// Free browser entries
void VideoBrowser_freeEntries(VideoBrowserContext* ctx) {
    if (ctx->entries) {
        free(ctx->entries);
        ctx->entries = NULL;
    }
    ctx->entry_count = 0;
}

// Compare function for sorting entries (directories first, then alphabetical)
static int compare_entries(const void* a, const void* b) {
    const VideoFileEntry* ea = (const VideoFileEntry*)a;
    const VideoFileEntry* eb = (const VideoFileEntry*)b;

    // Directories come first
    if (ea->is_dir && !eb->is_dir) return -1;
    if (!ea->is_dir && eb->is_dir) return 1;

    // Alphabetical (case-insensitive)
    return strcasecmp(ea->name, eb->name);
}

// Load directory contents (video files + directories)
void VideoBrowser_loadDirectory(VideoBrowserContext* ctx, const char* path, const char* root) {
    VideoBrowser_freeEntries(ctx);

    strncpy(ctx->current_path, path, sizeof(ctx->current_path) - 1);
    ctx->current_path[sizeof(ctx->current_path) - 1] = '\0';
    ctx->selected = 0;
    ctx->scroll_offset = 0;

    // Create video folder if it doesn't exist and we're at root
    if (strcmp(path, root) == 0) {
        mkdir(path, 0755);
    }

    DIR* dir = opendir(path);
    if (!dir) {
        LOG_error("Failed to open directory: %s\n", path);
        return;
    }

    // First pass: count entries
    int dir_count = 0;
    int video_count = 0;
    struct dirent* ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.') continue;  // Skip hidden files

        char full_path[1024];
        int path_len = snprintf(full_path, sizeof(full_path), "%s/%s", path, ent->d_name);
        if (path_len < 0 || path_len >= (int)sizeof(full_path)) {
            continue;  // Path too long, skip
        }

        struct stat st;
        if (stat(full_path, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            dir_count++;
        } else if (VideoBrowser_isVideoFile(ent->d_name)) {
            video_count++;
        }
    }

    int count = dir_count + video_count;

    // Add parent directory entry if not at root
    bool has_parent = (strcmp(path, root) != 0);
    if (has_parent) count++;

    // Allocate entries
    ctx->entries = malloc(sizeof(VideoFileEntry) * (count > 0 ? count : 1));
    if (!ctx->entries) {
        closedir(dir);
        return;
    }

    int idx = 0;

    // Add parent directory entry
    if (has_parent) {
        strncpy(ctx->entries[idx].name, "..", sizeof(ctx->entries[idx].name) - 1);
        ctx->entries[idx].name[sizeof(ctx->entries[idx].name) - 1] = '\0';
        char* last_slash = strrchr(ctx->current_path, '/');
        if (last_slash) {
            strncpy(ctx->entries[idx].path, ctx->current_path, last_slash - ctx->current_path);
            ctx->entries[idx].path[last_slash - ctx->current_path] = '\0';
        } else {
            strncpy(ctx->entries[idx].path, root, sizeof(ctx->entries[idx].path) - 1);
            ctx->entries[idx].path[sizeof(ctx->entries[idx].path) - 1] = '\0';
        }
        ctx->entries[idx].is_dir = true;
        ctx->entries[idx].format = VIDEO_FORMAT_UNKNOWN;
        idx++;
    }

    // Second pass: fill entries
    rewinddir(dir);
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.') continue;

        char full_path[1024];
        int path_len = snprintf(full_path, sizeof(full_path), "%s/%s", path, ent->d_name);
        if (path_len < 0 || path_len >= (int)sizeof(full_path)) {
            continue;
        }

        struct stat st;
        if (stat(full_path, &st) != 0) continue;

        bool is_dir = S_ISDIR(st.st_mode);
        VideoFormat fmt = VIDEO_FORMAT_UNKNOWN;

        if (!is_dir) {
            fmt = VideoBrowser_detectFormat(ent->d_name);
            if (fmt == VIDEO_FORMAT_UNKNOWN) continue;
        }

        strncpy(ctx->entries[idx].name, ent->d_name, sizeof(ctx->entries[idx].name) - 1);
        ctx->entries[idx].name[sizeof(ctx->entries[idx].name) - 1] = '\0';
        strncpy(ctx->entries[idx].path, full_path, sizeof(ctx->entries[idx].path) - 1);
        ctx->entries[idx].path[sizeof(ctx->entries[idx].path) - 1] = '\0';
        ctx->entries[idx].is_dir = is_dir;
        ctx->entries[idx].format = fmt;
        idx++;
    }

    closedir(dir);

    // Sort entries (keep ".." at top if present)
    int sort_start = has_parent ? 1 : 0;
    if (idx > sort_start + 1) {
        qsort(&ctx->entries[sort_start], idx - sort_start,
              sizeof(VideoFileEntry), compare_entries);
    }

    ctx->entry_count = idx;
}

// Get display name for file (without extension)
void VideoBrowser_getDisplayName(const char* filename, char* out, int max_len) {
    strncpy(out, filename, max_len - 1);
    out[max_len - 1] = '\0';

    // Remove extension for video files
    char* dot = strrchr(out, '.');
    if (dot && dot != out) {
        *dot = '\0';
    }
}

// Count video files in browser
int VideoBrowser_countVideoFiles(const VideoBrowserContext* ctx) {
    int count = 0;
    for (int i = 0; i < ctx->entry_count; i++) {
        if (!ctx->entries[i].is_dir) count++;
    }
    return count;
}

// Check if browser has parent entry (..)
bool VideoBrowser_hasParent(const VideoBrowserContext* ctx) {
    return ctx->entry_count > 0 && strcmp(ctx->entries[0].name, "..") == 0;
}

// Find subtitle file matching video
// Given /path/movie.mp4, checks for /path/movie.srt, .ass, .ssa, .sub
bool VideoBrowser_findSubtitle(const char* video_path, char* sub_path, int sub_path_size) {
    if (!video_path || !sub_path || sub_path_size <= 0) return false;

    // Find the base path without extension
    char base[512];
    strncpy(base, video_path, sizeof(base) - 1);
    base[sizeof(base) - 1] = '\0';

    char* dot = strrchr(base, '.');
    if (dot) {
        *dot = '\0';
    }

    // Try each subtitle extension
    static const char* sub_exts[] = {
        SUB_EXT_SRT,
        SUB_EXT_ASS,
        SUB_EXT_SSA,
        SUB_EXT_SUB,
        NULL
    };

    for (int i = 0; sub_exts[i] != NULL; i++) {
        snprintf(sub_path, sub_path_size, "%s.%s", base, sub_exts[i]);
        if (access(sub_path, F_OK) == 0) {
            return true;
        }
    }

    sub_path[0] = '\0';
    return false;
}
