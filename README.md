# The Catbox: Hacking Your Canon Cat for Great Justice

[Another Old VCR Pox Upon The Populace!](https://oldvcr.blogspot.com/2024/07/pretty-pictures-bootable-floppy-disks.html)

Copyright 2024 Cameron Kaiser.  
All rights reserved.  
BSD license.

## What it is

[The Canon Cat](https://oldvcr.blogspot.com/2024/05/refurb-weekend-canon-cat.html) is, officially, a "work processor" using a word processor-like interface to create and print documents, save and load them to and from disk, and send and receive them over the network. The machine was released in 1987 and sold for only about six months. It was designed by Jef Raskin, the original father of the Macintosh, and represents an unusual cul-de-sac in user experience design with its extremely keyboard-centric interface.

Nevertheless, regardless of how Canon marketed the device, it is a general purpose computer with a 68000 CPU running an operating system written largely in tForth, a token-threaded Forth dialect. Using available technical documentation it is possible to create disks with custom code: instead of simply containing documents, they can carry Forth and potentially even machine language binaries which automatically execute when the Cat loads from its internal floppy drive. This project shows you how.

This project contains three examples and utilities for working with disk images to patch and analyse them. [See the examples in action.](https://oldvcr.blogspot.com/2024/07/pretty-pictures-bootable-floppy-disks.html)

## What you need

* A Canon Cat, of course. MAME, the only extant Cat emulator, does not support the serial port or disk images yet.
  * Your Cat should have a working floppy drive and [you should shore up the guard rail](https://oldvcr.blogspot.com/2024/05/refurb-weekend-canon-cat.html) before you begin. There are ways to connect a PC drive, though you will have to [build a circuit and convert](https://groups.google.com/g/canon-cat/c/8AVearz7VgQ/m/LNuLBDz1fR8J) from the 34-pin PC floppy connector to the Cat's 20-pin floppy connector. No one has yet demonstrated using a floppy emulator with the Cat. Challenge accepted?
  * Configure your Cat for Forth mode. From a clean start with an empty document, enter `Enable Forth Language`, highlight it with the LEAP keys and press USE FRONT-ANSWER. A Forth icon will appear in the ruler briefly and the Cat will beep/blink the ruler. Next, press SHIFT-USE FRONT-SPACE, hit RETURN a couple times, and at the `ok` prompt enter `-1 wheel! savesetup re` and press RETURN. This will enable to you to enter and exit Forth at will with SHIFT-USE FRONT-SPACE as long as your settings battery stays charged.
* A [Greaseweazle](https://github.com/keirf/greaseweazle) floppy disk imaging tool and a 3.5" PC floppy drive capable of writing 720K floppies (HD-only drives will _not_ work). Other imagers may work if they can emit a [CPC eDSK-compatible image](https://www.cpcwiki.eu/index.php/Format:DSK_disk_image_file_format), but I use a Greaseweazle and the examples assume you are using one too. I paired it with an off-the-shelf Teac FD-235, which can write 720K disks.
* The utilities in this project are largely written in Perl, because I'm one of _those_ people, so you need Perl (5.8 or later). For `sendfile.c`, you of course need a C compiler and POSIX `termios`. It should build on pretty much any modern system.
* To transfer Forth or other data from your desktop to the Cat, you will need a serial cable with a DB-25 male connector on the Cat end to DE-9 ("DB-9") female connector on your workstation's end. This cable should be wired _straight-thru_, _not_ null modem. On your workstation side connect it directly or with your favourite USB-serial dongle as appropriate. In most cases you want 9600bps, 8-N-1, with the Cat's serial communications settings configured to use the "SEND Command" over a "Full Duplex" connection using CR/LF.

## Reading and writing Cat disk images

This project has selected the Amstrad CPC [Extended DSK image format](https://www.cpcwiki.eu/index.php/Format:DSK_disk_image_file_format) ("eDSK") as the archival storage format. This image format is high-level enough to allow easy manipulation, but sufficiently low-level to capture the nuances of the Cat's floppy layout, and is open-source, well-documented and well-supported. Greaseweazle's host tools support this format directly. The Cat writes single density MFM disks, one head, 80 tracks per disk with 10 sectors per track (numbered 0-9) and 512 bytes per sector, for a total storage capacity of 400K.

A Cat disk can be read with a command like

```
gw read --format=ibm-scan --tracks=c=0-79:h=0 test.edsk
```

which reads MFM sectors on tracks 0-79 with a single head and emits them in eDSK format to `test.edsk`. Tracks with no flux data are not emitted to the image and remain unformatted. Conversely, an eDSK image can be used to master a Cat floppy with a command like

```
gw write test.edsk
```

which writes the disk image directly using the eDSK's metadata for geometry and layout.

To make captures as clean as possible, it is recommended to use new disks and ensure they are newly formatted on the Cat (see the Cat's HELP-DISK screen for more information).

Cat floppy drives are notoriously problematic. Even working drives are known to have issues, and it would not be unexpected in drives this old to see alignment or track zero sensor failures. This may cause a disk image seemingly captured with no errors to nevertheless fail to master a bootable disk. The `optedsk` tool provided in this project can compensate for some causes of this (see below).

## Working with disk images (`catrpic`, `catwpic`, `catcpic`)

The `catrpic` tool reads the preview screenshot from a Cat eDSK image and emits it on standard output as a Netpbm Portable Bit Map (PBM) image. This image is inverted and rendered at 672x344, the Cat's native resolution. The other two utilities accept a PBM in this format, including the output of `catrpic` directly. Examples:

```
./catrpic test.edsk > test.pbm
```

The `catwpic` tool takes a Cat eDSK image and a suitable PBM image (in the format `catrpic` would generate) and merges them, emitting the hybrid eDSK to standard output. This image then becomes the preview image. It does not modify the text or other parts of the disk. Examples:

```
./catwpic test.edsk test.pbm > new.edsk
```

The `catcpic` tool takes a Cat eDSK image and a suitable PBM image (in the format `catrpic` would generate) and also merges them, emitting the hybrid eDSK to standard output. As part of the merging process it inserts a small 68K machine language subroutine and tForth payload which is run as the Cat loads from disk. **This code is only tested with, and only expected to work on, machines with production v1.74 ROMs. USE FRONT-HELP can tell you what version your Cat has.** The specific payload and its functionality depend on the arguments passed:

* If only an eDSK and PBM image are given, the Cat will pause during loading to display `press any key` in the lower right of the screen in reverse video, beep, and wait for the user to press a key after which loading will finish.
* If an eDSK, a PBM image and a four-digit zero-padded hexadecimal value corresponding to the target tForth token is passed, the Cat will run that token at the conclusion of loading but prior to entering the editor. This word must be present in the Forth dictionary in the source eDSK. After the token exits, the payload will move the user into the editor as usual, or the token definition can do so itself. The token is the value given with the `'` (tick) word, e.g., `' myword .` will print the token value to run the word `myword`.
* If an eDSK, a PBM image and a four-digit zero-padded hexadecimal value corresponding to the target tForth token is passed, as well as the `-cold` option, the Cat will run that token at the conclusion of loading with the expectation the target token will take over the system completely (a "cold booter") and will not enter the editor. If the token does exit, the machine will be cold-started with the `cold` word. As above, this word must be present in the Forth dictionary in the source eDSK, and the token is the value given with the `'` (tick) word, e.g., `' myword .` will print the token value to run the word `myword`.

`catcpic` creates a new random idtable so that the Cat will invariably recognize the disk as different from its current workspace. If you want to ensure the disk is written with a specific idtable, pass it as a 256-digit hexadecimal string using `-idtable=XXX` where `XXX` is the string. You are warned this may cause the Cat editor to be confused about what workspace is present on the resulting image.

Examples:

```
./catcpic test.edsk test.pbm > new.edsk
./catcpic test.edsk test.pbm 07e2 > new.edsk
./catcpic -idtable=01234[...]cdef test.edsk test.pbm 07e2 > new.edsk
./catcpic -cold test.edsk test.pbm 07e2 > new.edsk
./catcpic -cold -idtable=01234[...]cdef test.edsk test.pbm 07e2 > new.edsk
```

## Repairing defective or problematic images (`optedsk`)

In some cases the disk you make may not be readable on a real Cat. Common issues with mastering new floppies can be:

* The Cat drive has dirty heads. Any 3.5" floppy cleaning kit should be satisfactory.
* The Cat drive is misaligned and cannot read floppies written with another drive. The [Cat workshop manual](http://www.canoncat.net/) can suggest how to realign the drive. This assumes that _your_ PC drive is not misaligned itself, of course.
* The Cat drive that created the original image had a defective track zero sensor. This manifests as logical track zero being written at unpredictable locations, so when the image is mastered to a new floppy, logical track zero may not be at physical track zero and the disk will not be recognized as a Cat disk. It may be possible to edit the original eDSK to correct this problem.
  * On the other hand, if the Cat trying to read the _new_ disk has a defective track zero sensor, the workshop manual should be consulted to see if it can be calibrated.

The latter problem and certain other issues with eDSK images can be corrected with the `optedsk` tool. This tool takes an eDSK image, analyses it and corrects it if possible, and emits the corrected image on standard output. The corrected image ensures there is an idblock track always at physical track zero, fixes the track count if needed, and relocates subsequent tracks as necessary.

Analysis and correction are halted if multiple valid idblock tracks are found, formatted tracks are found before the valid idblock track (_unformatted_ tracks are automatically removed), or the disk does not appear to be a Cat disk.

* If the `-test` argument is passed, all operations will be done _except_ to emit the corrected image. This lets you see what `optedsk` thinks about your just-captured image, for example.
* If the `-force` argument is passed, analysis continues even if formatted tracks are found before the valid idblock track or the disk is missing the Cat identification sequence. In this mode `optedsk` will also try to remove preceding formatted tracks as well and relocate the idblock track to track zero that way, making all other necessary changes. **This mode is not well-tested and you may lose data.** `-force` does nothing if the disk does not appear to have Cat physical geometry (e.g., too many tracks, written with multiple heads, etc.), or if multiple valid idblock tracks are present. You will need to manually edit the eDSK to fix those issues.
* `-force -test` is valid in combination.

Examples:

```
./optedsk test.edsk > new.edsk
./optedsk -test test.edsk
./optedsk -force test.edsk > new.edsk
./optedsk -force -test test.edsk
```

## Other utilities (`fdis`, `interleave_rom`, `sendfile.c`)

`fdis` can disassemble tForth words from a ROM image (currently only the v1.74 ROMs are supported), passing it the filename of the interleaved ROM and either an address (decimal, or hexadecimal starting with `0x`) or a symbol, which will be looked up for you. The ROM file can be generated from the individual EPROM dumps, such as those used with MAME, using `interleave_rom`:

```
./interleave_rom r74*.ic[24] > rom
./interleave_rom r74*.ic[35] >> rom
```

After that, assuming your interleaved ROM is in the file `rom`, you can use commands like

```
./fdis rom "<save>"
```

which dumps the compiled definition of the word `<save>`. Disassembly terminates on the ending semicolon or equivalent token. You can see all words in the ROM file by passing the `-w` option, or look [at the pregenerated list](words).

`fdis` cannot currently disassemble words written purely in 68K assembly language (it will merely say that the word is native code and abort), and it will notify you of stub words that simply call `next`. Symbols and tokens directly referencing runtime variables cannot be disassembled either. Opcodes `fdis` cannot resolve in any known tForth vocabulary will be shown as question marks.

`sendfile` is a utility to send a file over a serial port at a specified speed, optionally with delays inserted after a configurable number of characters. It is written in C and requires a C compiler and POSIX `termios`. Compile it with (here using `gcc`):

```
gcc -O2 -o sendfile sendfile.c
```

It is tested on macOS and Linux. It requires the path to the serial port, the port speed and the filename to send, such as `./sendfile /dev/ttyUSB0 9600 file.txt`. If you optionally pass a comma-separated set of two numbers, such as `./sendfile /dev/ttyUSB0 9600 file.txt 1,10`, it will interpret that as the number of characters in a block followed by the number of milliseconds to pause after such a block (in this case, wait 10 milliseconds after every character). When sending text to the Cat over its serial port, `1,10` seems to be the quickest means that doesn't drop characters randomly.

`sendfile` connections are 8-N-1 (that is, eight bits, no parity and one stop bit), are assumed to be 8-bit safe, and neither use flow control nor any file transfer protocol. 

## Included Cat software

In [the `examples/` folder](examples/) are three demonstrations written with these tools: a Jef Raskin picture disk with his picture and a browseable copy of his Wikipedia entry; a simple Cat asynchronous terminal program; and a slideshow demo on two disks that boots from one and displays images written as tracks from the other. Ready-to-write disk images are included. Here's [how the demos were created](https://oldvcr.blogspot.com/2024/07/pretty-pictures-bootable-floppy-disks.html).

If you don't have a Cat, you can [watch videos of the examples in action](https://oldvcr.blogspot.com/2024/07/pretty-pictures-bootable-floppy-disks.html).

## Pull requests, feature requests and bug reports

Feature requests without a matching pull request are subject to closure and/or removal.

Bug reports without a matching pull request may or may not be addressed, even if valid and verifiable -- ever.

Pull requests are welcomed, but purely cosmetic changes, substantial refactors (without a reason I approve of) or ports to other programming languages are not. Please fork the project for those.

## License

Copyright (C) 2024 Cameron Kaiser. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE, SO THERE.