: cattermloop begin
		ser.?rxrdy if
			@ser.char
			demit
		then
		<?k> if
			char char? off record dup
			e1 = if
				leave
			then
			semit
		then
	0 until ;
: cattermhi cls home ." CatTerm oldvcr.blogspot.com UNDO to quit" cr ;
: catterm edde if 0 40f884 ! -1 410078 ! cattermhi cattermloop 0 410078 ! -1 40f884 ! new-display else cattermhi cattermloop then ;
