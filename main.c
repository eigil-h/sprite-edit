#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/window.h>
#include <proto/layout.h>
#include <proto/button.h>
#include <proto/integer.h>
#include <proto/getscreenmode.h>
#include <proto/getfile.h>
#include <reaction/reaction.h>
#include <reaction/reaction_macros.h>
#include <stdlib.h>
#include <stdio.h>
#include "datatypes.h"
#include "canvas.h"
#include "turbo.h"

/*
 * Public data
 */
struct Library
	*WindowBase,
	*RequesterBase,
	*LayoutBase,
	*ButtonBase,
	*GetScreenModeBase,
	*GetFileBase,
	*IntegerBase;

/*
 * Private data
 */
enum
{
	WIN_MAIN,    // main window
	WIN_GSM,     // screen mode requester
	WIN_GFR,     // open file requester
	WIN_GFW,     // save file requester
	WIN_PLT,		 // palette requester
	BTN_NEW,     // a new screen, screen mode requester
	BTN_OPEN,    // open a picture, file requester
	BTN_SAVE,    // save picture, file requester
	BTN_PALETTE, // open palette requester
	INT_PEN,     // selected pen display
	OBJ_SIZEOF
};

static Object* obj[OBJ_SIZEOF];
static MsgPort* app_port;
static Window* main_window;
static Screen* canvas_screen = NULL;

/*
 * Private protos
 */
static VOID init_libs(VOID);
static BOOL main_window_event_handler(ULONG*);
static VOID exit_handler(VOID);
static VOID pen_change_handler(UBYTE);

/*
 * Public functions
 */
int main(void)
{
	atexit(exit_handler);

	init_libs();

	if(app_port = CreateMsgPort()) // for eg AREXX messages
	{
		obj[WIN_MAIN] = WindowObject,
			WA_ScreenTitle, "Sprite Edit",
			WA_Title, "Small Picture Edit",
			WA_Activate, TRUE,
			WA_DepthGadget, TRUE,
			WA_DragBar, TRUE,
			WA_CloseGadget, TRUE,
			WA_SizeGadget, FALSE,
			WA_Width, 300,
			WA_Height, 40,
			WINDOW_IconifyGadget, TRUE,
			WINDOW_IconTitle, "Small Edit",
			WINDOW_AppPort, app_port,
			WINDOW_Position, WPOS_TOPLEFT,
			WINDOW_ParentGroup, HLayoutObject,
				LAYOUT_SpaceOuter, TRUE,
				LAYOUT_DeferLayout, TRUE,
				LAYOUT_AddChild, obj[BTN_NEW] = ButtonObject,
					GA_ID, BTN_NEW,
					GA_RelVerify, TRUE,
					GA_Text, "_New",
				ButtonEnd,
				LAYOUT_AddChild, obj[BTN_OPEN] = ButtonObject,
					GA_ID, BTN_OPEN,
					GA_RelVerify, TRUE,
					GA_Text, "_Open",
				ButtonEnd,
				LAYOUT_AddChild, obj[BTN_SAVE] = ButtonObject,
					GA_ID, BTN_SAVE,
					GA_RelVerify, TRUE,
					GA_Text, "_Save",
				ButtonEnd,
				LAYOUT_AddChild, obj[BTN_PALETTE] = ButtonObject,
					GA_ID, BTN_PALETTE,
					GA_RelVerify, TRUE,
					GA_Text, "_Palette",
				ButtonEnd,
				LAYOUT_AddChild, obj[INT_PEN] = IntegerObject,
					GA_ID, INT_PEN,
					INTEGER_Arrows, FALSE,
				IntegerEnd,
			EndGroup,
		EndWindow;


		obj[WIN_GSM] = GetScreenModeObject,
			GA_ID, WIN_GSM,
			GA_RelVerify, TRUE,
			GETSCREENMODE_TitleText, "Canvas screen mode",
			GETSCREENMODE_DoWidth, TRUE,
			GETSCREENMODE_DoHeight, TRUE,
			GETSCREENMODE_DoDepth, TRUE,
			GETSCREENMODE_DisplayWidth, 16,
			GETSCREENMODE_DisplayHeight, 16,
			GETSCREENMODE_MaxWidth, 64,
			GETSCREENMODE_MaxHeight, 64,
			GETSCREENMODE_MaxDepth, 4,
		GetScreenModeEnd;

		obj[WIN_GFR] = GetFileObject,
			GA_ID, WIN_GFR,
			GA_RelVerify, TRUE,
			GETFILE_TitleText, "Open file",
			GETFILE_ReadOnly, TRUE,
		GetFileEnd;

		obj[WIN_GFW] = GetFileObject,
			GA_ID, WIN_GFW,
			GA_RelVerify, TRUE,
			GETFILE_TitleText, "Save file",
			GETFILE_ReadOnly, FALSE,
			GETFILE_DoSaveMode, TRUE,
		GetFileEnd;

		if(main_window = (struct Window*) RA_OpenWindow(obj[WIN_MAIN]))
		{
			// ULONG app = 1L << app_port->mp_SigBit;
			WinTurbo turbo = {0};

			GetAttr(WINDOW_SigMask, obj[WIN_MAIN], &turbo.signal);

			turbo.event_handler = main_window_event_handler;

			main_turbo(&turbo);

			RA_CloseWindow(obj[WIN_MAIN]);
		}
	}

	return 0;
}

static void init_libs(void)
{
	if(!(WindowBase = OpenLibrary("window.class", -1)))
		exit(-1);

	if(!(RequesterBase = OpenLibrary("requester.class", -1)))
		exit(-2);

	if(!(LayoutBase = OpenLibrary("gadgets/layout.gadget", -1)))
		exit(-3);

	if(!(ButtonBase = OpenLibrary("gadgets/button.gadget", -1)))
		exit(-4);

	if(!(GetScreenModeBase = OpenLibrary("gadgets/getscreenmode.gadget", -1)))
		exit(-5);

	if(!(GetFileBase = OpenLibrary("gadgets/getfile.gadget", -1)))
		exit(-6);

	if(!(IntegerBase = OpenLibrary("gadgets/integer.gadget", -1)))
		exit(-7);
}

static void pen_change_handler(UBYTE pen)
{
	SetGadgetAttrs(
		(VOID*) obj[INT_PEN],
		main_window,
		NULL,
		INTEGER_Number,
		pen,
		TAG_DONE
	);
}

static BOOL main_window_event_handler(ULONG* signal)
{
	ULONG result;
	UWORD code;
	BOOL is_done = FALSE;

	while((result = RA_HandleInput(obj[WIN_MAIN], &code)) != WMHI_LASTMSG)
	{
		switch(result & WMHI_CLASSMASK)
		{
			case WMHI_CLOSEWINDOW:
				is_done=TRUE;
				break;

			case WMHI_GADGETUP:
				switch(result & WMHI_GADGETMASK)
				{
					case BTN_NEW:
						close_canvas();
						canvas_screen = NULL;

						if(RequestScreenMode(obj[WIN_GSM], main_window))
						{
							ULONG display_id;
							ULONG depth, width, height;

							GetAttr(GETSCREENMODE_DisplayID, obj[WIN_GSM],
								&display_id);

							GetAttr(GETSCREENMODE_DisplayWidth, obj[WIN_GSM],
								&width);

							GetAttr(GETSCREENMODE_DisplayHeight, obj[WIN_GSM],
								&height);

							GetAttr(GETSCREENMODE_DisplayDepth, obj[WIN_GSM],
								&depth);

							canvas_screen = open_canvas(
								display_id,
								width,
								height,
								depth,
								main_window->WScreen,
								pen_change_handler
							);
						}
						break;

					case BTN_OPEN:
						if(canvas_screen)
						{
							if(gfRequestFile(obj[WIN_GFR], main_window))
							{
								STRPTR filename;
								GetAttr(GETFILE_FullFile, obj[WIN_GFR], (ULONG*) &filename);
							}
						}
						else
						{
							DisplayBeep(main_window->WScreen);
						}
						break;

					case BTN_SAVE:
						if(canvas_screen)
						{
							if(gfRequestFile(obj[WIN_GFW], main_window))
							{
								STRPTR filename;
								GetAttr(GETFILE_FullFile, obj[WIN_GFW], (ULONG*) &filename);
							}
						}
						else
						{
							DisplayBeep(main_window->WScreen);
						}

						break;

					case BTN_PALETTE:
						DisplayBeep(main_window->WScreen);
						break;
					}
				break;

			case WMHI_ICONIFY:
				close_canvas();
				canvas_screen = NULL;
      	RA_Iconify(obj[WIN_MAIN]);
        main_window = NULL;
        break;

 			case WMHI_UNICONIFY:
      	if(main_window = (Window*) RA_OpenWindow(obj[WIN_MAIN]))
      	{
         	GetAttr(WINDOW_SigMask, obj[WIN_MAIN], signal);
      	}
				else
        {
        	is_done = TRUE;
        }
        break;
		}
	}

	return is_done;
}

static void exit_handler(void)
{
	if(app_port) DeleteMsgPort(app_port);

	if(IntegerBase) CloseLibrary(IntegerBase);
	if(GetFileBase) CloseLibrary(GetFileBase);
	if(GetScreenModeBase) CloseLibrary(GetScreenModeBase);
	if(ButtonBase) CloseLibrary(ButtonBase);
	if(LayoutBase) CloseLibrary(LayoutBase);
	if(RequesterBase) CloseLibrary(RequesterBase);
	if(WindowBase) CloseLibrary(WindowBase);
}
