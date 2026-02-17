#ifndef __FFPLAY_ENGINE_H__
#define __FFPLAY_ENGINE_H__

#include <stdbool.h>

typedef enum {
    FFPLAY_SOURCE_LOCAL,
    FFPLAY_SOURCE_STREAM,
} FfplaySourceType;

typedef struct {
    char path[2048];          // Video file path or stream URL (YouTube URLs can be very long)
    char subtitle_path[512];  // External subtitle file (empty = none)
    char title[256];          // Window title (empty = use filename/URL)
    char decryption_key[64];  // ClearKey hex string for DASH DRM (empty = none)
    int start_position_sec;   // Seek position (0 = start)
    FfplaySourceType source;  // LOCAL or STREAM
    bool is_stream;           // Stream mode flag
} FfplayConfig;

// Play a video using ffplay subprocess
// This function releases PAD, forks ffplay, waits for it to exit, then re-initializes PAD.
// Returns the ffplay exit code (0 = normal exit, non-zero = error)
int FfplayEngine_play(FfplayConfig* config);

// Stop the currently running ffplay process (if any)
void FfplayEngine_stop(void);

#endif
