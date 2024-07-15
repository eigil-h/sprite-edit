#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/window.h>
#include <proto/layout.h>
#include <proto/button.h>
#include <proto/getscreenmode.h>
#include <reaction/reaction.h>
#include <reaction/reaction_macros.h>
#include <stdlib.h>
#include <stdio.h>
#include "canvas.h"

enum
{
	WIN_MAIN,    // main window
	WIN_GSM,     // screen mode requester
	BTN_NEW,     // a new screen, screen mode requester
	BTN_OPEN,    // open a picture, file requester
	BTN_SAVE,    // save picture, file requester
	BTN_PALETTE, // open palette requester
	OBJ_SIZEOF
};

struct Library
	*WindowBase,
	*RequesterBase,
	*LayoutBase,
	*ButtonBase,
	*GetScreenModeBase; 

static struct MsgPort* app_port;

static void init_libs(void);
static void exit_handler(void);

int main(void)
{
	atexit(exit_handler);

	init_libs();

	if(app_port = CreateMsgPort())
	{
		Object* obj[OBJ_SIZEOF];
		struct Window* window;

		obj[WIN_MAIN] = WindowObject,
			WA_ScreenTitle, "Sprite Edit",
			WA_Title, "Sprite Edit Control Window",
			WA_Activate, TRUE,
			WA_DepthGadget, TRUE,
			WA_DragBar, TRUE,
			WA_CloseGadget, TRUE,
			WA_SizeGadget, FALSE,
			WA_Width, 300,
			WA_Height, 40,
			WINDOW_IconifyGadget, TRUE,
			WINDOW_IconTitle, "Sprite Edit",
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
			EndGroup,
		EndWindow;


		obj[WIN_GSM] = GetScreenModeObject,
			GA_ID, WIN_GSM,
			GA_RelVerify, TRUE,
			GETSCREENMODE_TitleText, "Canvas screen mode",
			GETSCREENMODE_DoWidth, TRUE,
			GETSCREENMODE_DoHeight, TRUE,
			GETSCREENMODE_DoDepth, TRUE,
		GetScreenModeEnd;

		if(window = (struct Window*) RA_OpenWindow(obj[WIN_MAIN]))
		{
			ULONG app = 1L << app_port->mp_SigBit;
			BOOL done = FALSE;
			ULONG signal;

			GetAttr(WINDOW_SigMask, obj[WIN_MAIN], &signal);

			while(!done)
			{
				ULONG wait = Wait( signal | SIGBREAKF_CTRL_C | app);

				if(wait & SIGBREAKF_CTRL_C)
				{
					done = TRUE;
				}
				else
				{
					ULONG result;
					UWORD code;

					while((result = RA_HandleInput(obj[WIN_MAIN], &code)) !=
						WMHI_LASTMSG)
					{
						switch(result & WMHI_CLASSMASK)
						{
							case WMHI_CLOSEWINDOW:
								done=TRUE;
								break;

							case WMHI_GADGETUP:
								switch(result & WMHI_GADGETMASK)
								{
									case BTN_NEW:
										close_screen();

										if(RequestScreenMode(obj[WIN_GSM], window))
										{
											ULONG display_id;
											ULONG depth;

											GetAttr(GETSCREENMODE_DisplayID, obj[WIN_GSM],
												 &display_id);

											GetAttr(GETSCREENMODE_DisplayDepth, obj[WIN_GSM],
												 &depth);

											open_screen(display_id, 0, 0, depth);
										}
										break;
									case BTN_OPEN:
										break;
									case BTN_SAVE:
										break;
									case BTN_PALETTE:
										break;
								}
								break;

       				case WMHI_ICONIFY:
             		RA_Iconify(obj[WIN_MAIN]);
             		window = NULL;
             		break;

        			case WMHI_UNICONIFY:
              	if(window = (struct Window *) RA_OpenWindow(obj[WIN_MAIN]))
             		{
                	GetAttr(WINDOW_SigMask, obj[WIN_MAIN], &signal);
             		}
								else
             		{
                	done = TRUE;
             		}
            		break;
						}
					}
				}
			}
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
}

static void exit_handler(void)
{
	if(app_port) DeleteMsgPort(app_port);

	if(GetScreenModeBase) CloseLibrary(GetScreenModeBase);
	if(ButtonBase) CloseLibrary(ButtonBase);
	if(LayoutBase) CloseLibrary(LayoutBase);
	if(WindowBase) CloseLibrary(WindowBase);
}
