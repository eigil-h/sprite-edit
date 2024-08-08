#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <stdlib.h>
#include <stdio.h>
#include "datatypes.h"
#include "turbo.h"
#include "canvas.h"

/*
 * Private data
 */
static Screen* screen;
static Window* preview_window;
static Window* edit_window;
static WinTurbo edit_turbo;
static Window* palette_window;
static WinTurbo palette_turbo;
static OnPenChanged on_pen_changed;

/*
 * Private protos
 */
static BOOL palette_input_handler(ULONG*);
static VOID draw_border(Window*);
static VOID draw_palette(VOID);
static UBYTE palette_columns(UBYTE screen_depth);
static UBYTE palette_rows(UBYTE screen_depth);
static WORD find_pen(WORD x, WORD y);
static BOOL edit_input_handler(ULONG*);

/*
 * Public functions
 */
VOID open_canvas(
	ULONG display_id,
	UWORD width,
	UWORD height,
	UBYTE depth,
	OnPenChanged opc)
{
	LONG error;

	atexit(close_canvas);

	on_pen_changed = opc;

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
		WA_RMBTrap, TRUE,
		TAG_DONE
	))) exit(-1);

	draw_border(preview_window);

	if(!(edit_window = OpenWindowTags(NULL,
		WA_CustomScreen, (ULONG) screen,
		WA_Left, (screen->Width >> 1) - (2 * width),
		WA_Width, 4 * width,
		WA_Height, 4 * height,
		WA_Backdrop, TRUE,
		WA_Borderless, TRUE,
		WA_Activate, TRUE,
		WA_RMBTrap, TRUE,
		WA_IDCMP, IDCMP_MOUSEBUTTONS,
		TAG_DONE
	))) exit(-1);

	draw_border(edit_window);

	edit_turbo.signal = 1 << edit_window->UserPort->mp_SigBit;
	edit_turbo.port = edit_window->UserPort;
	edit_turbo.event_handler = edit_input_handler;

	add_win_turbo(&edit_turbo);

	if(!(palette_window = OpenWindowTags(NULL,
		WA_CustomScreen, (ULONG) screen,
		WA_Left, screen->Width - (screen->Width >> 3),
		WA_Width, (screen->Width >> 3) - 1,
		WA_Height, screen->Height >> 1,
		WA_Backdrop, TRUE,
		WA_Borderless, TRUE,
		WA_Activate, FALSE,
		WA_RMBTrap, TRUE,
		WA_IDCMP, IDCMP_MOUSEBUTTONS,
		TAG_DONE
	))) exit(-1);

	draw_palette();

	palette_turbo.signal = 1 << palette_window->UserPort->mp_SigBit;
	palette_turbo.port = palette_window->UserPort;
	palette_turbo.event_handler = palette_input_handler;

	add_win_turbo(&palette_turbo);
}

VOID close_canvas(VOID)
{
	if(palette_window)
	{
		remove_win_turbo(&palette_turbo);
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

static BOOL palette_input_handler(ULONG* signal)
{
	IntuiMessage* message;
	ULONG class;
	UWORD code;
	WORD mouse_x, mouse_y;
	WORD pen;

	while(message = (IntuiMessage*) GetMsg(palette_turbo.port))
	{
		class = message->Class;
		code = message->Code;
		mouse_x = message->MouseX;
		mouse_y = message->MouseY;

		ReplyMsg((Message*) message);


		switch(class)
		{
			case IDCMP_MOUSEBUTTONS:
				switch(code)
				{
					case SELECTDOWN:
						pen = find_pen(mouse_x, mouse_y);
						SetAPen(edit_window->RPort, (UBYTE) pen);
						on_pen_changed((UBYTE) pen);
						break;
				}
				break;
		}
	}

	return FALSE;
}

static WORD find_pen(WORD x, WORD y)
{
	UBYTE screen_depth = palette_window->RPort->BitMap->Depth;
	UBYTE columns = palette_columns(screen_depth),
		rows = palette_rows(screen_depth);
	WORD cell_width = (palette_window->Width + (columns-1)) / columns;
	WORD cell_height = palette_window->Height / rows;
  WORD col = x / cell_width;
  WORD row = y / cell_height;

  return (WORD) (row * columns + col);
}

static BOOL edit_input_handler(ULONG* signal)
{
	IntuiMessage* message;
	ULONG class;
	UWORD code;
	WORD mouse_x, mouse_y;

	while(message = (IntuiMessage*) GetMsg(edit_turbo.port))
	{
		class = message->Class;
		code = message->Code;
		mouse_x = message->MouseX >> 2 << 2;
		mouse_y = message->MouseY >> 2 << 2;

		ReplyMsg((Message*) message);


		switch(class)
		{
			case IDCMP_MOUSEBUTTONS:
				switch(code)
				{
					case SELECTDOWN:
						// Move(edit_window->RPort, mouse_x, mouse_y);
						RectFill(
							edit_window->RPort, 
							mouse_x, 
							mouse_y,
							mouse_x + 3,
							mouse_y + 3
						);
						break;

					case SELECTUP:
						// Draw(edit_window->RPort, mouse_x, mouse_y);
						break;
				}
				break;
		}
	}

	return FALSE;
}
