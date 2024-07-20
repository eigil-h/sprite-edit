#ifndef APP_CANVAS_H
#define APP_CANVAS_H

#include <intuition/screens.h>

typedef struct Screen Screen;
typedef struct Window Window;
typedef VOID (*OnPenChanged)(UBYTE);

VOID open_canvas(
	ULONG display_id,
	UWORD width,
	UWORD height,
	UBYTE depth,
	OnPenChanged);
VOID close_canvas(VOID);

#endif
