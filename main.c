#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/window.h>
#include <proto/layout.h>
#include <proto/button.h>
#include <proto/alib.h>
#include <reaction/reaction.h>
#include <reaction/reaction_macros.h>
#include <gadgets/layout.h>
#include <gadgets/button.h>

enum ButtonId {
	NEW_ID,
	OPEN_ID,
	SAVE_ID,
	PALETTE_ID
};

int main(void)
{
	struct Library *WindowBase, *LayoutBase, *ButtonBase;
	struct MsgPort* app_port;

	WindowBase = OpenLibrary("window.class", -1);
	LayoutBase = OpenLibrary("gadgets/layout.gadget", -1);
	ButtonBase = OpenLibrary("gadgets/button.gadget", -1);

	if(app_port = CreateMsgPort())
	{
		Object *new_button, *open_button, *save_button, *palette_button;
		Object*	win_obj = WindowObject,
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
			WINDOW_ParentGroup, VLayoutObject,
				LAYOUT_SpaceOuter, TRUE,
				LAYOUT_DeferLayout, TRUE,
				LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
				LAYOUT_AddChild, new_button = ButtonObject,
					GA_ID, NEW_ID,
					GA_RelVerify, TRUE,
					GA_Text, "_New",
				ButtonEnd,
				LAYOUT_AddChild, open_button = ButtonObject,
					GA_ID, OPEN_ID,
					GA_RelVerify, TRUE,
					GA_Text, "_Open",
				ButtonEnd,
				LAYOUT_AddChild, save_button = ButtonObject,
					GA_ID, SAVE_ID,
					GA_RelVerify, TRUE,
					GA_Text, "_Save",
				ButtonEnd,
				LAYOUT_AddChild, palette_button = ButtonObject,
					GA_ID, PALETTE_ID,
					GA_RelVerify, TRUE,
					GA_Text, "_Palette",
				ButtonEnd,
			EndGroup,
		EndWindow;
		struct Window* window = (struct Window*) RA_OpenWindow(win_obj);

		if(window)
		{
			ULONG app = 1L << app_port->mp_SigBit;
			BOOL done = FALSE;
			ULONG signal;

			GetAttr(WINDOW_SigMask, win_obj, &signal);

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

					while((result = RA_HandleInput(win_obj, &code)) != WMHI_LASTMSG)
					{
						switch(result & WMHI_CLASSMASK)
						{
							case WMHI_CLOSEWINDOW:
								done=TRUE;
								break;

							case WMHI_GADGETUP:
								break;

       				case WMHI_ICONIFY:
             		RA_Iconify(win_obj);
             		window = NULL;
             		break;

        			case WMHI_UNICONIFY:
              	if(window = (struct Window *) RA_OpenWindow(win_obj))
             		{
                	GetAttr(WINDOW_SigMask, win_obj, &signal);
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
			RA_CloseWindow(win_obj);
		}
		DeleteMsgPort(app_port);
	}

	if(ButtonBase) CloseLibrary(ButtonBase);
	if(LayoutBase) CloseLibrary(LayoutBase);
	if(WindowBase) CloseLibrary(WindowBase);

	return 0;
}
