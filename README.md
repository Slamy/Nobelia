# Nobelia

Real 210/05
427
429
57

Turbo
737
749
304

Normal
352
377
171

Normal Nochmal
527
555
61



## Compiling under Linux

### Prerequisites

Clone https://github.com/TwBurn/cdi-sdk and have it mounted as D: drive in winecfg.

Get [tocs12.zip](http://www.icdia.co.uk/sw_pc/vcdtools.html) if you want to convert
the resulting image to CUE/BIN for usage with MAME and MiSTer.
It is expected to have tocsplit.exe in the root of this project.

### Compiling

	WINEPATH=D:/DOS/BIN wine D:/dos/bin/bmake.exe link_app
	WINEPATH=D:/DOS/BIN wine D:/dos/bin/bmake.exe link_cd

### Starting via serial stub

	wine /home/andre/Downloads/cdistub-0.5.1/cdilink/cdilink.exe cdilink -port 5 -n -a 8000 -d build/colortst.app -e && minicom -D /dev/ttyUSB0 -b 9600

### Cleanup

	WINEPATH=D:/DOS/BIN wine D:/dos/bin/bmake.exe clean

### Mastering into CDI/TOC file

	dosbox master.bat -exit

The resulting image can be loaded into cdiemu

### Conversion into CUE/BIN

You need to do that to use the image in MAME. Keep in mind that the tool
requires mouse control.

	wine tocsplit.exe

### Compiling, mastering and CUE/BIN conversion in one step

This approach is crude and might not work on all machines.
It makes use of xdotool to automate button presses.

	./make_image.sh 

### Start image on MAME

	mame cdimono1 -cdrom disk/COLORTST.CUE

### Start image on cdiemu

	wine wcdiemu-v053b7.exe disk/COLORTST.CDI
	wine wcdiemu-v053b8.exe COLORTST.CDI -playcdi -start -term uart


### Copy to MiSTer

	scp disk/COLORTST.CUE disk/COLORTST.BIN root@mister:/media/fat/games/CD-i

## CD-i Game by Jeffrey Janssen - nobelia@nmotion.nl

This is the Open Source version of Nobelia. 
- The source code is identical to the released version.
- Licensed music is not included in this version, all in-game music tracks are replaced by "The Traveller"
- The source code and assets in this repository are meant for personal and/or educational use only

### Controls

**Up/Down/Left/Right**
- Move character

**Button 1**
- Start game on titlescreen
- Place bomb (tile in front of you needs to be empty)
- Activate chests/switches
- Restart level after death

**Button 2**
- Detonate remote bombs (when you have this powerup)

**Button 3**
- Pause game

## Resources

All assets in the game were either used from Open resources or created/adapted by me.

- Titlescreen:
	- Image: http://clipart-library.com/clipart/348852.htm
	- Music: Grassy World by Matthew Pablo: https://opengameart.org/content/grassy-world-overture-8bitorchestral
- Level sprites:
	- Armando Montero (ArMM1998): https://opengameart.org/content/zelda-like-tilesets-and-sprites
- Level Music
	- The Traveller by Viktor Kraus: https://opengameart.org/content/the-traveller
- Character Sprites:
	- Lanea Zimmerman (Sharm), Evert, withthelove: https://opengameart.org/content/tiny-16-expanded-character-sprites

## Changelog
- Makefile has been updated to use the compiler/toolset available from https://github.com/TwBurn/cdi-sdk
- Use DOSBox to call `vcdmastr.exe` since this is a 16-bit application (see `master.bat`)