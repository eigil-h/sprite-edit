#include <proto/intuition.h>
#include <proto/graphics.h>
#include <stdlib.h>
#include "canvas.h"

/*
 * Private data
 */
static Screen* screen = NULL;

/*
 * Public functions
 */
VOID close_screen(VOID)
{
	if(screen)
	{
		CloseScreen(screen);
		screen = NULL;
	}
}

VOID open_screen(
	ULONG display_id,
	UWORD width,
	UWORD height,
	UBYTE depth)
{
	atexit(close_screen);

	screen = OpenScreenTags(NULL,
		SA_DisplayID, display_id,
		SA_Left, 0,
		SA_Top, 100,
//		SA_Width, width,
//		SA_Height, height,
		SA_Depth, depth,
		SA_Title, (ULONG) "Canvas",
		SA_Type, CUSTOMSCREEN,
		SA_Interleaved, TRUE,
		SA_BitMap, NULL,
		SA_SysFont, 1,
		TAG_DONE
	);
}
