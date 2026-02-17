#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#include "vp_defines.h"
#include "api.h"
#include "msettings.h"
#include "ffplay_engine.h"

// PID of the currently running ffplay child process (0 = none)
static pid_t ffplay_pid = 0;

// Check if Bluetooth audio is active (via msettings or .asoundrc)
static int is_bluetooth_audio(void) {
    if (GetAudioSink() == AUDIO_SINK_BLUETOOTH) return 1;
    const char* home = getenv("HOME");
    if (home) {
        char path[512];
        snprintf(path, sizeof(path), "%s/.asoundrc", home);
        FILE* f = fopen(path, "r");
        if (f) {
            char buf[256];
            while (fgets(buf, sizeof(buf), f)) {
                if (strstr(buf, "bluealsa")) { fclose(f); return 1; }
            }
            fclose(f);
        }
    }
    return 0;
}

// Build argv for ffplay and exec in a forked child. Returns exit status.
static int ffplay_exec(FfplayConfig* config, int use_subs) {
    char* argv[32];
    int argc = 0;

    argv[argc++] = FFPLAY_PATH;
    argv[argc++] = "-fs";         // Fullscreen
    argv[argc++] = "-autoexit";   // Exit when video ends
    argv[argc++] = "-loglevel";
    argv[argc++] = "error";

    // Seek position
    char seek_str[32];
    if (config->start_position_sec > 0) {
        snprintf(seek_str, sizeof(seek_str), "%d", config->start_position_sec);
        argv[argc++] = "-ss";
        argv[argc++] = seek_str;
    }

    // Subtitle filter
    char vf_str[1024];
    if (use_subs && config->subtitle_path[0] != '\0') {
        snprintf(vf_str, sizeof(vf_str), "subtitles='%s'", config->subtitle_path);
        argv[argc++] = "-vf";
        argv[argc++] = vf_str;
    }

    // Window title
    if (config->title[0] != '\0') {
        argv[argc++] = "-window_title";
        argv[argc++] = config->title;
    }

    // Stream buffering options
    if (config->is_stream) {
        argv[argc++] = "-infbuf";       // Disable buffer size limit for live streams
        argv[argc++] = "-framedrop";    // Drop frames if decoding too slow
        argv[argc++] = "-probesize";
        argv[argc++] = "5000000";       // 5MB probe size
        argv[argc++] = "-analyzeduration";
        argv[argc++] = "5000000";       // 5 seconds analysis
        argv[argc++] = "-user_agent";   // YouTube CDN requires a browser User-Agent
        argv[argc++] = "Mozilla/5.0";
    }

    // ClearKey decryption for DASH DRM streams (CENC)
    if (config->decryption_key[0] != '\0') {
        argv[argc++] = "-cenc_decryption_key";
        argv[argc++] = config->decryption_key;
    }

    // Input file (must be last)
    argv[argc++] = "-i";
    argv[argc++] = config->path;
    argv[argc] = NULL;

    // Set up Bluetooth audio routing before fork
    int bt_audio = is_bluetooth_audio();
    if (bt_audio) {
        // Set BlueALSA mixer to 100% so audio is audible
        system("amixer scontrols 2>/dev/null | grep -i 'A2DP' | "
               "sed \"s/.*'\\([^']*\\)'.*/\\1/\" | "
               "while read ctrl; do amixer sset \"$ctrl\" 127 2>/dev/null; done");
    }

    // Fork and exec ffplay
    ffplay_pid = fork();
    if (ffplay_pid < 0) {
        LOG_error("fork() failed: %s\n", strerror(errno));
        return -1;
    }

    if (ffplay_pid == 0) {
        // Child process: set audio device for SDL2 inside ffplay
        if (bt_audio)
            setenv("AUDIODEV", "bluealsa", 1);
        execv(FFPLAY_PATH, argv);
        _exit(127);
    }

    // Parent process: wait for ffplay to exit
    int status = 0;
    int result;
    do {
        result = waitpid(ffplay_pid, &status, 0);
    } while (result == -1 && errno == EINTR);

    ffplay_pid = 0;

    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        if (code != 0) {
            LOG_error("ffplay exited with code %d, url: %s\n", code, config->path);
        }
        return code;
    }
    return -1;
}

int FfplayEngine_play(FfplayConfig* config) {
    if (!config || config->path[0] == '\0') {
        return -1;
    }

    // Check if ffplay binary exists before doing anything
    if (access(FFPLAY_PATH, X_OK) != 0) {
        LOG_error("ffplay binary not found: %s\n", FFPLAY_PATH);
        return -1;
    }

    LOG_info("ffplay: playing %s\n", config->path);

    // Release joysticks so ffplay can use them for input.
    // Do NOT call GFX_quit() - it has a double-free bug in shared code
    // (overlay_path + system fonts). ffplay will open /dev/fb0 independently
    // via its own SDL2; both can mmap the framebuffer simultaneously.
    PAD_quit();

    int exit_code = ffplay_exec(config, config->subtitle_path[0] != '\0');

    // Re-initialize input and clear stale button states
    PAD_init();
    PAD_reset();

    return exit_code;
}

void FfplayEngine_stop(void) {
    if (ffplay_pid > 0) {
        kill(ffplay_pid, SIGTERM);
        // Give it a moment to clean up
        usleep(100000);
        // Force kill if still running
        kill(ffplay_pid, SIGKILL);
        waitpid(ffplay_pid, NULL, WNOHANG);
        ffplay_pid = 0;
    }
}
