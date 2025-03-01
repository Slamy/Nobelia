#!/bin/bash

# Abort on error
set -e

# Clear files to be sure that we have generated them
rm -f disk/*.BIN build/*.CDI disk/*.CUE disk/*.TOC build/MASTER.LOG

# First build the application
time WINEPATH=D:/DOS/BIN wine D:/dos/bin/bmake.exe link_cd

# Then start MS-DOS to master the image
# Use a custom configuration to overclock the machine
# Use dummy video driver to hide the window
time SDL_VIDEODRIVER=dummy dosbox -conf dosbox.conf master.bat -exit

# The error codes are not available on MSDOS. We check for the log to be sure
cat build/MASTER.LOG
grep "End   generation of album" build/MASTER.LOG

# Convert the CDI/TOC files into CUE/BIN using mouse automation

wine tocsplit.exe&
sleep 0.2
WID=$(xdotool search --name --sync tocsplit)
xdotool windowactivate --sync $WID
# Click "Open TOC"
xdotool mousemove --sync -w $WID 220 340
xdotool click -w $WID 1
# Double-Click on first image
sleep 0.2
xdotool mousemove --sync -w $WID 40 40
xdotool click --repeat 2 --delay 50 1
# Click on GO!
xdotool mousemove --sync -w $WID 320 340
xdotool click -w $WID 1
sleep 0.2
# Click on Exit
xdotool mousemove --sync -w $WID 500 340
xdotool click -w $WID 1

ls -lh disk/*.CUE disk/*.BIN

sed -i -e "s/FILE .*\\\\/FILE /" disk/*.CUE

echo " --- Done! ---"
