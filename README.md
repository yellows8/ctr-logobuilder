This repo is for building the 3DS NCCH ExeFS:/logo file. This file contains *absolutely* *everything* that gets displayed on the logo screens when Home Menu is booting an regular application.

To keep it simple, only one image per screen is used, without any actual animation. See the scripts and the main tool for details.

"prebuilt_homebrew_logo-padded.lz11" is a prebuilt homebrew logo binary which you can use with the makerom -logo option. The "hblogo_{bottom|top}*" files in this .git directory are the texture files used with this prebuilt logo file.

For the build scripts, the first param is the prefix file-path for the logo files, while the second param is the darc directory path. That directory must contain the following sub-directories: "anim", "blyt", and "timg". The timg directory must contain the following BCLIM files, which can be the output from ctr-gputextool(https://github.com/yellows8/ctr-gputextool) for example: "hblogo_bottom.bclim"(bottom-screen) and "hblogo_top.bclim"(top-screen).

The utils.* and types.h files are from ctrtool.

Requirements for using the build scripts:
* https://github.com/yellows8/darctool
* https://github.com/Steveice10/bannertool (lz11 compression)
* "$HOME/.3ds/logo_hmackey_text", hex 64-character text file, originally extracted from binary in the Home Menu codebin.

The following documentation were referenced as a base:
* http://florian.nouwt.com/wiki/index.php/CLYT_%28File_Format%29
* http://wiibrew.org/wiki/Wii_Animations

