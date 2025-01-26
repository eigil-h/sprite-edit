#ifndef APP_CANVAS_H
#define APP_CANVAS_H

#include <intuition/screens.h>

typedef VOID (*OnPenChanged)(UBYTE);

Screen* open_canvas(
	ULONG display_id,
	UWORD width,
	UWORD height,
	UBYTE depth,
	Screen* parent,
	OnPenChanged);
VOID close_canvas(VOID);
BOOL load(CONST_STRPTR);
BOOL save(CONST_STRPTR);

#endif
