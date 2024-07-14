This is a simple asynchronous terminal program for the Cat. The ready-to-write disk image can be used to master a Cat floppy disk with:

```
gw write catterm.edsk
```

Insert the disk into the Cat and turn it on, or from the editor with a blank workspace or after saving the current workspace, insert the disk and press USE FRONT-DISK, or type `cold` at the Forth `ok` prompt.

The terminal program is a dummy terminal with no terminal emulation. XON/XOFF is used for flow control. Terminal settings are as configured in SETUP. There is no local echo.

To exit the terminal program, press UNDO. By default this returns to the editor, and you can run it from the editor by entering `catterm`, highlighting it with the LEAP keys, and pressing USE FRONT-ANSWER. You can also run it from the Forth `ok` prompt with `catterm`, upon which it will return to the `ok` prompt on termination instead.

The source code is in `catterm.forth` (CR/LF line feeds) and can be passed to the Cat using `sendfile` with a 10ms intercharacter delay (e.g., `sendfile /dev/ttyUSB0 9600 catterm.forth 1,10`). The disk image `orig_catterm_07e2.edsk` contains this text both raw and compiled to tForth opcodes, and `catterm.pbm` was extracted from it. The `catterm.edsk` final image was processed with `../../catcpic orig_catterm_07e2.edsk catterm.pbm 07e2 > catterm.edsk`.

Copyright (C)2024 Cameron Kaiser. All rights reserved. BSD license.
