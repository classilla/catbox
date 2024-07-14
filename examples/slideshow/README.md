This is a slideshow demonstration for the Cat on two disks. The Cat boots from the first disk and pauses for the second disk to be inserted, after which it displays a sequence of images until the UNDO key is pressed. The ready-to-write disk images can be used to master each Cat floppy disk with:

```
gw write slideshow.edsk
gw write slideshow_2.edsk
```

Insert the first disk into the Cat and turn it on, or from the editor with a blank workspace or after saving the current workspace, insert the disk and press USE FRONT-DISK, or type `cold` at the Forth `ok` prompt. When the blinking question mark appears, eject the first disk and insert the second. The demo will run until you press the UNDO key, after which the Cat will restart into the editor.

The source code is in `slideshow.forth` (CR/LF line feeds) and can be passed to the Cat using `sendfile` with a 10ms intercharacter delay (e.g., `sendfile /dev/ttyUSB0 9600 slideshow.forth 1,10`). The disk image `orig_slideshow_07e4.edsk` contains this text both raw and compiled to tForth opcodes. The `slideshow.edsk` final image was processed with `../../catcpic orig_slideshow_07e4.edsk slideshow.pbm 07e4 > slideshow.edsk`.

The credits screen was created by entering text into the Cat, saving it, capturing the disk as `catdtxt.edsk`, and extracting the preview image as a PBM with `../../catrpic catdtxt.edsk > catdtxt.pbm`.

The image tracks on the second disk image were written using this program at the Forth `ok` prompt:

```
ser.xon.tx.off 3c 0 do 4070e0 400000 do ser.rx i c! loop 400000 i wtrk 401400 i 1 + wtrk 402800 i 2 + wtrk 403c00 i 3 + wtrk 40500 i 4 + wtrk 405ce0 i 5 + wtrk . . . . . . 6 +loop
```

Each image is then transmitted in order using `sendfile`. An intercharacter delay is not needed for this purpose. After the image is transmitted, wait for the Cat to complete writing this image's tracks, and then send the next image. This code supports 10 images, not counting the preview image. The disk image `slideshow_2.edsk` contains these tracks and does not require post-processing.

Copyright (C)2024 Cameron Kaiser. All rights reserved. BSD license.
