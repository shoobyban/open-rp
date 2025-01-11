Open Remote Play
================

https://github.com/shoobyban/open-rp

Previously: https://code.google.com/p/open-rp/

Originally forked from [@dsokoloski](https://github.com/dsokoloski)

Wouldn't it be cool if you could run Remote Play from your laptop, desktop, or
perhaps your phone?

After 2 weeks of hardcore reverse-engineering, now you can!

I would like to thank Dark_AleX for hacking the PSP.  Without his work, ORP
would not be possible.  I would also like to thank TyRaNiD for psplink and
prxtool.  Using psplink makes this type of work almost painless!  Thanks to
jas0nuk for the essential PRXdecrypter utility.

Thanks to Greg for the MacBook loan and beer!

Thanks to MohammadAG for the virtual keyboard idea!

Controls
--------

If you connect your SIXAXIS or DualShock 3 controller to your PC *before* you
start Open Remote Play, it should be detected and automatically configured.
Remember to press the PlayStation button to activate the controller.

For Windows users, you require to download and install the DualShock driver:
http://www.motioninjoy.com/

Ensure that the controller is functioning properly by testing it using the
Game Controllers program found in the Control Panel.

The player currently implements these keyboard bindings:

    ENTER        X
    SPACE        Triangle
    F1           Triangle
    F2           Square
    F3           Select
    F4           Start
    ESCAPE       Circle
    PAGE-UP      L1
    PAGE-DOWN    R1
    ARROW KEYS   D-Pad
    HOME         PS Button
    BACKSPACE    Square

    CTRL-1       Select bit-rate: 1024k
    CTRL-2       Select bit-rate: 768k
    CTRL-3       Select bit-rate: 384k
    CTRL-D       Toggle through window sizes (normal, medium, and large)
    CTRL-F       Enter/exit full-screen mode
    CTRL-S       Toggle the status OSD on/off
    CTRL-Q       Quit Remote Play

Virtual keyboard:
NOTE: This is a hack and is bound to be buggy!

While you are in a PS3 text-input dialog, you can enter virtual keyboard mode
using CTRL-ENTER.  The window caption will be appended with "Virtual Keyboard
Mode".  While in this mode, keys pressed on your real keyboard are translated
in to D-Pad presses followed by X to speed up text entry.  The right and left
arrow keys send L1/R1 and the ENTER key sends a line-break instead of X.  Always
ensure that the letter 'g' is high-lighted *before* entering virtual keyboard
mode, or after you've moved the cursor manually.

To exit virtual keyboard mode, press CTRL-ENTER again.

There are no keyboard bindings for the following controller input:
The right/left analog sticks, L2, R2, L3, and R3.

NOTE: On Apple laptops, you may have to hold down the Fn key to access some of
the above keys.

The player currently implements the following mouse support:

While in full-screen mode, mouse motion deltas are sent as PSP D-Pad events
while the ALT key is held down.  In window-mode, mouse motion does nothing.
The mouse buttons are mapped the same as when you plug a USB mouse in to the
PS3:

    LEFT BUTTON     X
    RIGHT BUTTON    Triangle
    MIDDLE BUTTON   Home

    WHEEL UP        D-Pad Up
    WHEEL DOWN      D-Pad Down

Configuration
-------------

You require a PlayStation Portable (or access to one), that is able to run
homebrew software.

After installing the application on your PC, you need to import existing RP
settings from a registered PSP.  The steps are as follows:

1.  If you have not done so already, register your PSP with your PS3 for
    Remote Play.

2.  Copy the ORP_Export folder to your PSP under /PSP/GAME

3.  Run Open Remote Play Export on your PSP.  This will create a file on the
    memory stick called export.orp.  Copy this file to your PC.

4.  Run the ORP GUI and click on Import, locate and import export.orp.  You
    will have to manually change the IP address of your PS3, by default it
    will be set to 0.0.0.0.

    You can find your PS3's IP address from the XMB under:
    Settings > Network Settings > Setting & Connection Status List

5.  Click Import again, this time locate and import keys.orp.  You only have
    to do this once for a new installation, or if there is a key update.

6.  Select a profile, cross your fingers, and click Launch!

NOTE: You can duplicate a profile by changing the PS3 Name and clicking Save.
This is useful if you wish to keep two profiles, one for the local LAN, and
another for Internet connections.

Known Issues
------------

https://github.com/shoobyban/open-rp/issues

Change Log
-------------------------------------------------------------------------------

https://github.com/shoobyban/open-rp/commits/master

Compilation
-----------
TODO: Terribly incomplete, lacking in documentation here...

When cloning this repository, ensure you enable recursive mode to ensure any
required submodules (ex: FFMpeg) are included:

    # git clone --recursive git@github.com:shoobyban/open-rp.git

Running ./autogen.sh is required to create required build system files/scripts:

    # ./autogen.sh

To build against an alternate FFMPEG installation using pkg-config:

    # cd ./ffmpeg-build
    # ./configure --prefix=../ffmpeg-install [<your custom FFMpeg optoins>]
    # make && make install
    # cd ..
    # ./configure PKG_CONFIG_LIBDIR=./ffmpeg-install/lib/pkgconfig PKG_CONFIG_SYSROOT_DIR=./ffmpeg-build/ --prefix=/usr

To update the FFMpeg submodule to the master branch:

    # git submodule foreach git pull origin master
    # git commit -a

