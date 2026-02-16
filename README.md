# NextUI Video Player
A video playback application for NextUI featuring local file playback, YouTube streaming, IPTV/Online TV, and channel subscriptions.

## Supported Platforms
- **tg5040** - TrimUI Smart Pro / TrimUI Brick / Brick Hammer
- **tg5050** - TrimUI Smart Pro S

## Installation

### Manual Installation
1. Mount your NextUI SD card to a computer.
2. Download the latest release file named `Video.Player.pak.zip` from Github.
3. Copy the zip file to `/Tools/<PLATFORM>/Video.Player.pak.zip` (replace `<PLATFORM>` with your device: `tg5040` or `tg5050`).
4. Extract the zip in place, then delete the zip file.
5. Confirm that there is a `/Tools/<PLATFORM>/Video.Player.pak` folder on your SD card.
6. Rename the `Video.Player.pak` folder to `Video Player.pak`.
7. Unmount your SD Card and insert it into your TrimUI device.

### Pak Store Installation

1. Open `Pak Store` application in your TrimUI device.
2. Navigate to the `Browse` then `Media` menu.
3. Select `Video Player` to install.

## Update

1) You can update the application directly via `Settings > About` page in the application.
2) Or, you can update via `Pak Store`.

## Features

### General
- Automatic screen off (Follow system screen timeout).
- Resume playback from where you left off.
- Self-update from GitHub releases.

### Local Video Playback
- Supports `MP4`, `MKV`, `AVI`, `WEBM`, `MOV`, `FLV`, `M4V`, `WMV`, `MPEG`, `3GP` formats
- File browser for navigating video libraries (Video files must be placed in `./Videos` folder)
- Subtitle support (embedded SRT/ASS/SSA tracks in `.mkv` and other containers)
- On-screen display with progress bar, elapsed/total time

### YouTube Streaming
- Search YouTube videos directly from the device
- Thumbnail previews in search results
- Stream resolution selection
- Channel subscriptions (bookmarks)

### IPTV / Online TV
- M3U/M3U8 playlist support
- Bundled curated channel list
- Browse channels by category
- Add custom playlist URLs

## Controls

### Main Menu Navigation
- **D-Pad**: Navigate menus and file browser
- **A Button**: Select/Confirm
- **B Button**: Back/Cancel/Exit
- **Start (short press)**: Show Controls Help
- **Start (long press)**: Exit Application

### Video Player (during playback)
- **A Button**: Play/Pause
- **B Button**: Stop and return to browser
- **D-Pad Left/Right**: Seek ±10 seconds (hold for continuous seek)
- **D-Pad Up/Down**: Seek ±60 seconds (hold for continuous seek)

## Usage

### Playing Local Videos
- Navigate to your video folder using the `Local Videos` menu
- Select a file to start playback
- Place subtitle files (`.srt`, `.ass`) alongside video files with matching names for automatic detection

### YouTube
- Navigate to `YouTube` from the main menu
- Use the on-screen keyboard to search for videos
- Select a video to stream
- Press `Y` to subscribe to a channel

### IPTV
- Navigate to `Online TV` from the main menu
- Browse channels by category or flat list
- Select a channel to stream
- Add custom M3U playlist URLs

## Building from Source

### Prerequisites
- Docker (for cross-compilation toolchain)
- NextUI workspace with platform dependencies

### Build Commands

#### Building the Video Player application
```bash
# Enter the toolchain (replace PLATFORM accordingly)
make shell PLATFORM=tg5040

# Once in the toolchain shell
cd ~/workspace/nextui-video-player/src

# Build for TrimUI Brick (tg5040)
make clean && make PLATFORM=tg5040
```

#### Building ffplay (video playback engine)

ffplay is a patched build of FFmpeg's ffplay with gamepad input support and custom OSD. It is built separately from the main application.

```bash
# Build ffplay using Docker toolchain
docker run --rm -v $(pwd)/workspace:/root/workspace \
  ghcr.io/loveretro/tg5040-toolchain:latest /bin/bash -c \
  'source ~/.bashrc && bash /root/workspace/nextui-video-player/ffplay/build.sh'
```

The build script will:
1. Download FFmpeg 6.1 source (cached after first run)
2. Set up SDL2 headers and device's shared library for cross-compilation
3. Apply the patched `ffplay.c` (gamepad controls, OSD, progress bar)
4. Build and copy the resulting binary to `bin/ffplay`

### Project Structure

```
workspace/
├── nextui-video-player/        # This project
│   ├── src/                    # Source code
│   ├── ffplay/                 # ffplay build system
│   │   ├── ffplay.c            # Patched ffplay source (gamepad + OSD)
│   │   ├── build.sh            # Cross-compilation build script
│   │   ├── sdl2-headers/       # SDL2 headers for cross-compilation
│   │   └── syslibs/            # Device's SDL2 shared library
│   ├── bin/                    # Platform binaries and runtime tools
│   │   ├── tg5040/             # TrimUI Brick binary (videoplayer.elf)
│   │   ├── tg5050/             # TrimUI Smart Pro S binary (videoplayer.elf)
│   │   ├── ffplay              # Patched ffplay binary (shared across platforms)
│   │   ├── yt-dlp              # YouTube downloader
│   │   ├── wget                # HTTP downloader
│   │   └── keyboard            # On-screen keyboard
│   ├── res/                    # Resources (fonts, images)
│   ├── playlists/              # Bundled IPTV playlists
│   └── state/                  # Runtime state files
├── all/                        # Shared code
│   ├── common/                 # Common utilities, API
│   └── minarch/                # Emulator framework
├── tg5040/                     # TrimUI Brick platform
│   ├── platform/               # Platform-specific code
│   └── libmsettings/           # Settings library
└── tg5050/                     # TrimUI Smart Pro platform
    ├── platform/               # Platform-specific code
    └── libmsettings/           # Settings library
```

### Dependencies

The video player uses:
- **Shared code**: `workspace/all/common/` (utils, api, config, scaler)
- **Platform code**: `workspace/<PLATFORM>/platform/`
- **Libraries**: SDL2, SDL2_image, SDL2_ttf, GLESv2, EGL, libzip, mbedTLS
- **ffplay**: Patched FFmpeg 6.1 ffplay (dynamically linked against device's SDL2)
