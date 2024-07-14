( wait for a key )
: keywait ( delay - key )
	0 swap
	0 do
		drop
		<?k> if	
			char char? off leave
		then
		1 ms
		0
	loop ;

( wait until the disk is out )
: diskwaitout ( delay - flag )
	0 swap
	0 do
		drop
		?diskrdy not dup if
			leave
		then
		1 ms
	loop doff ;

( wait until the disk is in )
: diskwaitin ( delay - flag )
	0 swap
	0 do
		drop
		?diskrdy dup if
			leave
		then
		1 ms
	loop doff ;

( general disk wait loop )
: diskwait ( waitword )
	begin
		( erase question mark )
		403cdd 4038ed do ffff i w! 54 +loop
		( ensure drive is on, needs a delay )
		drive0 40 800000 or! 180 ms
		dup 0500 swap execute if drop leave then

		( draw question mark - bitmap data is lifo )
		fe7f fe7f ffff fe7f fe7f fe3f ff1f ff8f f3cf f3cf f00f f81f
			403cdd 4038ed do i w! 54 +loop
		drive0 40 800000 or! 180 ms
		dup 0500 swap execute if drop leave then
	0 until doff ;

( display a picture stored on disk at a particular track number )
: trackpic ( track - )
	( each picture is six tracks long with overlap in the last track )
	dup
	400000 swap rtrk drop
	dup 1 +
	401400 swap rtrk drop
	dup 2 +
	402800 swap rtrk drop
	dup 3 +
	403c00 swap rtrk drop
	dup 4 +
	405000 swap rtrk drop
	5 +
	405ce0 swap rtrk drop
	( wait for a keypress for 2560 ticks or reset if UNDO pressed )
	0a00 keywait e1 = if cold then
	;

( main loop )
: slideshow ( - )
	( turn off editor )
	edde off crt on

	( mac gimme disk screen is already encoded in the screenshot )
	['] diskwaitout diskwait
	['] diskwaitin diskwait

	( make sure we can read track 0 )
	recal 0 <> if
		page ." disk read failure"
		1000 keywait drop cold
	then

	( draw 32x32 happy mac and erase bottom of floppy disk icon )
	aaaaaaaa 50000015 a7ffffca 57ffffd5 a7ffffca 50000015 afffffea 4fffffe5 afffffea 4fffffe5 acffc0ea 4fffffe5 afffffea 4fffffe5 afffffea 4e0000e5 adffff6a 4dffff65 adf87f6a 4df7bf65 adffff6a 4dfcff65 adfeff6a 4dfeff65 adeeef6a 4deeef65 adffff6a 4dffff65 adffff6a 4e0000e5 afffffea 57ffffd5 a800002a
		403dd8 403304 do i ! 54 +loop

	( shocked mac slide, then loop the rest of the slideshow )
	400 ms 10 keywait drop ( prime key events ) 0 trackpic begin
		0a ( number of pictures, then multiplied by tracks per pic )
		6 * 6 do i trackpic 6 +loop ( each picture is six tracks )
	0 until ;

( shamelessness )
: credits ." copyright 2024 cameron kaiser" cr ;

