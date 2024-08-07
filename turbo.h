#ifndef APP_MAIN_TURBO_H
#define APP_MAIN_TURBO_H

#include "datatypes.h"

/*
 * Types
 */
typedef BOOL (*EventHandler)(ULONG* signal);

typedef struct
{
	MinNode node;
	ULONG signal;
	MsgPort* port;
	EventHandler event_handler;
} WinTurbo;

/*
 * Protos
 */
VOID main_turbo(WinTurbo*);
VOID add_win_turbo(WinTurbo*);
VOID remove_win_turbo(WinTurbo*);

#endif
