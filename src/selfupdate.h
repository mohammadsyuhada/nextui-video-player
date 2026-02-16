#ifndef __SELFUPDATE_H__
#define __SELFUPDATE_H__

#include <stdbool.h>

// GitHub repository (format: "owner/repo")
#define APP_GITHUB_REPO "mohammadsyuhada/nextui-video-player"

// Release asset name pattern (the .pak.zip file)
#define APP_RELEASE_ASSET "Video.Player.pak.zip"

// Fallback version if version file not found
#define APP_VERSION_FALLBACK "0.0.0"

// Self-update module states
typedef enum {
    SELFUPDATE_STATE_IDLE = 0,
    SELFUPDATE_STATE_CHECKING,
    SELFUPDATE_STATE_DOWNLOADING,
    SELFUPDATE_STATE_EXTRACTING,
    SELFUPDATE_STATE_APPLYING,
    SELFUPDATE_STATE_COMPLETED,
    SELFUPDATE_STATE_ERROR
} SelfUpdateState;

// Update status information
typedef struct {
    SelfUpdateState state;
    bool update_available;
    char current_version[32];
    char latest_version[32];
    char download_url[512];
    char release_notes[1024];
    int progress_percent;
    long download_bytes;
    long download_total;
    char status_detail[64];
    char status_message[256];
    char error_message[256];
} SelfUpdateStatus;

// Initialize self-update module
// pak_path: path to the .pak directory
int SelfUpdate_init(const char* pak_path);

// Cleanup resources
void SelfUpdate_cleanup(void);

// Get current app version
const char* SelfUpdate_getVersion(void);

// Check for updates (non-blocking, runs in background thread)
int SelfUpdate_checkForUpdate(void);

// Start the update process (download + extract + apply)
int SelfUpdate_startUpdate(void);

// Cancel ongoing update
void SelfUpdate_cancelUpdate(void);

// Get current status
const SelfUpdateStatus* SelfUpdate_getStatus(void);

// Update function (call from main loop)
void SelfUpdate_update(void);

// Check if restart is required to apply update
bool SelfUpdate_isPendingRestart(void);

// Get current state
SelfUpdateState SelfUpdate_getState(void);

#endif
