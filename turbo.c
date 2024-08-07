#include <proto/alib.h>
#include <proto/exec.h>
#include "turbo.h"

/*
 * Private data
 */
static MinList winturbo_list;

/*
 * Private protos
 */
ULONG get_signal_set(VOID);
BOOL handle_all(ULONG);

/*
 * Public functions
 */
VOID main_turbo(WinTurbo* winturbo)
{
	BOOL is_done = FALSE;

	INIT_LIST(winturbo_list);
	ADD_NODE(winturbo_list, winturbo);

	while(!is_done)
	{
		ULONG signal_set = get_signal_set();
		ULONG sig = Wait(signal_set);

		is_done = handle_all(sig);
	}
}

VOID add_win_turbo(WinTurbo* winturbo)
{
	ADD_NODE(winturbo_list, winturbo);
}

VOID remove_win_turbo(WinTurbo* winturbo)
{
	REMOVE_NODE(winturbo);
}

/*
 * Private functions
 */
ULONG get_signal_set(VOID)
{
	ULONG signal_set = 0L;
	MinNode* node;

  for(node = winturbo_list.mlh_Head; 
		node->mln_Succ != NULL;
		node = node->mln_Succ)
	{
		signal_set |= ((WinTurbo*) node)->signal;
	}

	return signal_set;
}

BOOL handle_all(ULONG sig)
{
	MinNode* node;
	BOOL is_done = FALSE;

  for(node = winturbo_list.mlh_Head; 
		node->mln_Succ != NULL;
		node = node->mln_Succ)
	{
		WinTurbo* wt = (WinTurbo*) node;

		if(sig & wt->signal)
		{
			is_done = wt->event_handler(&wt->signal);
		}
	}

	return is_done;
}
