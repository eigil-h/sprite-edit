#ifndef APP_DATATYPES_H
#define APP_DATATYPES_H

#include <exec/lists.h>
#include <exec/ports.h>
#include <graphics/gfx.h>
#include <intuition/intuition.h>
#include <datatypes/pictureclass.h>
#include <libraries/iffparse.h>

/*
 * macros
 */
#define INIT_LIST(list) (NewList((struct List*) &(list)))
#define ADD_NODE(list, nptr) (AddTail((struct List*) &(list), (struct Node*) (nptr)))
#define REMOVE_NODE(nptr) (Remove((struct Node*) (nptr)))

/*
 * system
 */
typedef struct MinList MinList;
typedef struct MinNode MinNode;
typedef struct MsgPort MsgPort;
typedef struct Screen Screen;
typedef struct Window Window;
typedef struct IntuiMessage IntuiMessage;
typedef struct Message Message;
typedef struct BitMap BitMap;
typedef struct BitMapHeader BitMapHeader;
typedef struct IFFHandle IFFHandle;

/*
 * custom
 */
typedef ULONG* Palette32;

typedef struct
{
	UWORD width, height, depth, pad;
	BitMap* bitmap;
	Palette32 palette;
} PictureData;

#endif
