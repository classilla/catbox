This is the "Jef Raskin picture disk" which contains a picture of Jef Raskin, the designer of the Cat, along with his Wikipedia entry which can be browsed using the LEAP keys. The ready-to-write disk image can be used to master a Cat floppy disk with:

```
gw write raskin.edsk
```

Insert the disk into the Cat and turn it on, or from the editor with a blank workspace or after saving the current workspace, insert the disk and press USE FRONT-DISK, or type `cold` at the Forth `ok` prompt.

The disk will load a picture of Raskin interacting with the Cat, pause for a key to be pressed, and then enter a read-only version of the document. Instructions appear at the beginning. This is a regular Cat document otherwise.

The Raskin image was resized and converted to PBM from a source PNG using ImageMagick (`magick raskinbw.png -resize 672x344\! -dither FloydSteinberg -remap pattern:gray50 -negate raskinbw.pbm`). The text of the article was rendered by Lynx from Wikipedia, then converted to CR/LF line endings and hand-edited. It was transferred to the Cat using `sendfile` and a 10ms intercharacter delay (e.g., `sendfile /dev/ttyUSB0 9600 raskinlf.txt 1,10`). The disk image `orig_raskin.edsk` contains this text. The `raskin.edsk` final image was processed with `../../catcpic orig_raskin.edsk raskinbw.pbm > raskin.edsk`.

Copyright (C)2024 Cameron Kaiser. All rights reserved. BSD license. The text of the Raskin article appears under separate license; see the end of `raskinlf.txt` or the Cat document for details.
