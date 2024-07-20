#include <proto/intuition.h>
#include <proto/graphics.h>
#include <stdlib.h>
#include <stdio.h>
#include "canvas.h"

/*
 * Private data
 */
static Screen* screen;
static Window* preview_window;
static Window* edit_window;
static Window* palette_window;

/*
 * Private protos
 */
static VOID draw_border(Window*);
static VOID draw_palette(VOID);
static UBYTE palette_columns(UBYTE screen_depth);
static UBYTE palette_rows(UBYTE screen_depth);

/*
 * Public functions
 */
VOID open_canvas(
	ULONG display_id,
	UWORD width,
	UWORD height,
	UBYTE depth,
	OnPenChanged on_pen_changed)
{
	LONG error;

	atexit(close_canvas);

	if(!(screen = OpenScreenTags(NULL,
		SA_DisplayID, display_id,
		SA_Left, 0,
		SA_Top, 100,
		SA_Depth, depth,
		SA_Interleaved, TRUE,
		SA_Type, CUSTOMSCREEN,
		SA_Interleaved, TRUE,
		SA_Title, "Small picture edit",
		SA_ShowTitle, TRUE,
		SA_Quiet, TRUE,
		SA_ErrorCode, (ULONG) &error,
		TAG_DONE
	))) exit(error);

	if(!(preview_window = OpenWindowTags(NULL,
		WA_CustomScreen, (ULONG) screen,
		WA_Width, width,
		WA_Height, height,
		WA_Backdrop, TRUE,
		WA_Borderless, TRUE,
		WA_Activate, FALSE,
		TAG_DONE
	))) exit(-1);

	draw_border(preview_window);

	if(!(edit_window = OpenWindowTags(NULL,
		WA_CustomScreen, (ULONG) screen,
		WA_Left, (screen->Width >> 1) - (4 * width),
		WA_Width, 4 * width,
		WA_Height, 4 * height,
		WA_Backdrop, TRUE,
		WA_Borderless, TRUE,
		WA_Activate, TRUE,
		TAG_DONE
	))) exit(-1);

	draw_border(edit_window);

	if(!(palette_window = OpenWindowTags(NULL,
		WA_CustomScreen, (ULONG) screen,
		WA_Left, screen->Width - (screen->Width >> 3),
		WA_Width, (screen->Width >> 3) - 1,
		WA_Height, screen->Height >> 1,
		WA_Backdrop, TRUE,
		WA_Borderless, TRUE,
		WA_Activate, FALSE,
		WA_IDCMP, IDCMP_MOUSEBUTTONS,
		TAG_DONE
	))) exit(-1);

	draw_palette();
}

VOID close_canvas(VOID)
{
	if(palette_window)
	{
		CloseWindow(palette_window);
		palette_window = NULL;
	}

	if(edit_window)
	{
		CloseWindow(edit_window);
		edit_window = NULL;
	}

	if(preview_window)
	{
		CloseWindow(preview_window);
		preview_window = NULL;
	}

	if(screen)
	{
		CloseScreen(screen);
		screen = NULL;
	}
}

/*
 * Private functions
 */
static VOID draw_border(Window* win)
{
	Move(win->RPort, 0, 0);
	Draw(win->RPort, 0, win->Height-1);
	Move(win->RPort, 0, 0);
	Draw(win->RPort, win->Width-1, 0);

	Move(win->RPort, win->Width-1, win->Height-1);
	Draw(win->RPort, 0, win->Height-1);
	Move(win->RPort, win->Width-1, win->Height-1);
	Draw(win->RPort, win->Width-1, 0);
}

static VOID draw_palette(VOID)
{
	UBYTE screen_depth = palette_window->RPort->BitMap->Depth;
	UBYTE pen = 0, x, y,
		columns = palette_columns(screen_depth),
		rows = palette_rows(screen_depth);
	WORD entry_height = palette_window->Height / rows;
	WORD entry_width = (palette_window->Width + (columns-1)) / columns;

	for(y=0; y < rows; y++)
	{
		for(x=0; x < columns; x++)
		{
			SetAPen(palette_window->RPort, pen++);
			RectFill(
				palette_window->RPort,
				x * entry_width,
				y * entry_height,
				((x+1) * entry_width) - 1,
				((y+1) * entry_height) - 1
			);
		}
	}
}

static UBYTE palette_columns(UBYTE screen_depth)
{
	if(screen_depth > 3)
		return (UBYTE) ((1 << screen_depth) >> 3);
	return (UBYTE) 1;
}

static UBYTE palette_rows(UBYTE screen_depth)
{
	if(screen_depth < 3)
		return (UBYTE) (1 << screen_depth);
	return (UBYTE) 8;
}
