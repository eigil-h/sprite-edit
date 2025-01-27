#include <proto/exec.h>
#include <proto/iffparse.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iow.h"

#define BYTES_PER_ROW(w,d) (((((ULONG)(w) + 15) >> 3) & 0xFFFE) * (d))

/*
 * Protos
 */
static VOID palette32_to_cmap(Palette32 palette, UBYTE* cmap_data, ULONG len);
static VOID bitmap_to_body(BitMap*, UBYTE*);
static VOID free_io(struct IFFHandle*);
static BOOL write_chunk(struct IFFHandle*, LONG id, APTR data, LONG size);

/*
 * Public
 */
BOOL save_picture(CONST_STRPTR path, const PictureData* pic)
{
	struct IFFHandle* iff_handle;
	BitMapHeader bmhd = {0};
	ULONG cmap_size = 3 * (1 << pic->depth);
	UBYTE* cmap = AllocMem(cmap_size, NULL);
	ULONG body_size = pic->bitmap->BytesPerRow * pic->bitmap->Rows;
	UBYTE* body = AllocMem(body_size, NULL);
	LONG result;

	bmhd.bmh_Width = pic->width;
	bmhd.bmh_Height = pic->height;
	bmhd.bmh_Depth = pic->depth;
	bmhd.bmh_Compression = 0;

	if(!cmap || !body) //todo: possible mem leak
	{
		return FALSE;
	}

	if(!(iff_handle = AllocIFF()))
	{
		return FALSE;
	}

	if(!(iff_handle->iff_Stream = Open(path, MODE_NEWFILE)))
	{
		free_io(iff_handle);
		return FALSE;
	}

	InitIFFasDOS(iff_handle);

	if(result = OpenIFF(iff_handle, IFFF_WRITE))
	{
		free_io(iff_handle);
		return FALSE;
	}

	if(result = PushChunk(iff_handle, ID_ILBM, ID_FORM, IFFSIZE_UNKNOWN))
	{
		free_io(iff_handle);
		return FALSE;
	}

	if(!write_chunk(iff_handle, ID_BMHD, &bmhd, sizeof(BitMapHeader)))
	{
		free_io(iff_handle);
		return FALSE;
	}

	palette32_to_cmap(pic->palette, cmap, cmap_size);

	if(!write_chunk(iff_handle, ID_CMAP, cmap, cmap_size))
	{
		free_io(iff_handle);
		return FALSE;
	}

	bitmap_to_body(pic->bitmap, body);

	if(!write_chunk(iff_handle, ID_BODY, body, body_size))
	{
		free_io(iff_handle);
		return FALSE;
	}

	if(result = PopChunk(iff_handle))
	{
		free_io(iff_handle);
		return FALSE;
	}

	FreeMem(body, body_size);
	FreeMem(cmap, cmap_size);
	free_io(iff_handle);

	return TRUE;
}

/*
 * Private
 */
static VOID palette32_to_cmap(Palette32 palette, UBYTE* cmap_data, ULONG len)
{
	int i;

	for(i = 0; i < len; i++)
	{
		*cmap_data++ = (*palette++) >> 24;
	}
}

static VOID bitmap_to_body(BitMap* bm, UBYTE* body)
{
	memcpy(body, bm->Planes[0], bm->BytesPerRow * bm->Rows);
}

static BOOL write_chunk(
	struct IFFHandle* iff_handle,
	LONG id,
	APTR data,
	LONG size
)
{
	LONG result;

	if(PushChunk(iff_handle, NULL, id, size))
	{
		return FALSE;
	}

	result = WriteChunkBytes(iff_handle, data, size);

	if(PopChunk(iff_handle))
	{
		return FALSE;
	}

	if(result != size)
	{
		return FALSE;
	}

	return TRUE;
}

static void free_io(struct IFFHandle* iff_handle)
{
	if(iff_handle) {
		CloseIFF(iff_handle);
		if(iff_handle->iff_Stream)
			Close(iff_handle->iff_Stream);
		FreeIFF(iff_handle);
	}
}
