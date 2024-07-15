#ifndef APP_CANVAS_H
#define APP_CANVAS_H

#include <intuition/screens.h>

typedef struct Screen Screen;

VOID open_screen(
	ULONG display_id,
	UWORD width,
	UWORD height,
	UBYTE depth);
VOID close_screen(VOID);

#endif
