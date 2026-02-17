#!/bin/sh
DIR="$(dirname "$0")"
cd "$DIR"

# Use system PLATFORM variable, fallback to tg5040 if not set
[ -z "$PLATFORM" ] && PLATFORM="tg5040"

export LD_LIBRARY_PATH="$DIR:$DIR/bin:$DIR/bin/$PLATFORM:/rom/usr/trimui/lib:/usr/lib:$LD_LIBRARY_PATH"

# Set CPU frequency for video player (needs higher ceiling for video processing)
echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null
echo 408000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq 2>/dev/null
echo 2000000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq 2>/dev/null

export SDL_NOMOUSE=1
export HOME=/mnt/SDCARD

# Create Videos directory if it doesn't exist
mkdir -p /mnt/SDCARD/Videos

# Run the platform-specific binary
"$DIR/bin/$PLATFORM/videoplayer.elf" &> "$LOGS_PATH/video-player.txt"
