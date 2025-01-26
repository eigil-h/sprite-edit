#include <proto/exec.h>
#include <proto/iffparse.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "io.h"

#define BYTES_PER_ROW(w,d) (((((ULONG)(w) + 15) >> 3) & 0xFFFE) * (d))

/*
 * Protos
 */
static VOID cmap_to_palette32(
  const UBYTE* cmap_data,
  const ULONG size,
  Palette32 palette,
	UBYTE depth
);
static VOID decompress(
	const UBYTE* source,
	UBYTE* destination,
	const size_t compressed_size,
	const size_t decompressed_size);
static VOID free_io(struct IFFHandle*);
static UWORD min(UWORD a, UWORD b);
static VOID body_to_bitmap(BitMapHeader*, UBYTE*, BitMap*);

/*
 * Public
 */
BOOL load_picture(CONST_STRPTR path, PictureData* picture_data)
{
	struct IFFHandle* iff_handle = NULL;
	BitMapHeader* bmhd = NULL;
	UBYTE* cmap = NULL;
	ULONG cmap_size;
	UBYTE* body = NULL;
	ULONG body_size;
	LONG error;

	if(!(iff_handle = AllocIFF())) {
		return NULL;
	}

	if(!(iff_handle->iff_Stream = Open(path, MODE_OLDFILE))) {
		free_io(iff_handle);
		return NULL;
	}

	InitIFFasDOS(iff_handle);

	if(error = OpenIFF(iff_handle, IFFF_READ)) {
		free_io(iff_handle);
		return NULL;
	}

	if(error = PropChunk(iff_handle, ID_ILBM, ID_BMHD)) {
		free_io(iff_handle);
		return NULL;
	}

	if(error = PropChunk(iff_handle, ID_ILBM, ID_CMAP)) {
		free_io(iff_handle);
		return NULL;
	}

	if(error = PropChunk(iff_handle, ID_ILBM, ID_BODY)) {
		free_io(iff_handle);
		return NULL;
	}

	StopOnExit(iff_handle, ID_ILBM, ID_FORM);

	if((error = ParseIFF(iff_handle, IFFPARSE_SCAN)) == IFFERR_EOC) {
		struct StoredProperty* sp;

		if(sp = FindProp(iff_handle, ID_ILBM, ID_BMHD)) {
			bmhd = (struct BitMapHeader*) sp->sp_Data;
		}

		if(sp = FindProp(iff_handle, ID_ILBM, ID_CMAP)) {
			cmap = (UBYTE *) sp->sp_Data;
			cmap_size = sp->sp_Size;
		}

		if(sp = FindProp(iff_handle, ID_ILBM, ID_BODY)) {
			body = (UBYTE *) sp->sp_Data;
			body_size = sp->sp_Size;
		}
	}

	if(bmhd && cmap && body) {
		if(bmhd->bmh_Masking == mskHasMask ||
			bmhd->bmh_Masking == mskHasAlpha) {
			// not supported until I understand what this is
		} else {
			BOOL body_compressed = (BOOL) bmhd->bmh_Compression;

			cmap_to_palette32(
				cmap,
				cmap_size,
				picture_data->palette,
				picture_data->depth);

			if(body_compressed) {
				decompress(
					body,
					picture_data->bitmap->Planes[0],
					body_size,
					bmhd->bmh_Depth *
						RASSIZE(bmhd->bmh_Width,
										bmhd->bmh_Height)
				);
			} else {
				body_to_bitmap(bmhd, body, picture_data->bitmap);
			}
		}
	}

	free_io(iff_handle);

	return TRUE;
}

BOOL save_picture(CONST_STRPTR path, PictureData* pic)
{
	return FALSE;
}

/*
 * Private
 */
static VOID cmap_to_palette32(
  const UBYTE* cmap_data,
  const ULONG cmap_size,
  Palette32 palette,
	UBYTE depth
)
{
	UWORD length = min(cmap_size, 3 * (1 << depth));
	unsigned i;

	*palette++ = (length/3) << 16;

	for (i = 0; i < length; i++) {
		*palette++ = (*cmap_data++ << 24) & 0xFFFFFFFF;
  }

	*palette = 0L;
}

static UWORD min(UWORD a, UWORD b)
{
	if(a <= b) return a;
	return b;
}

/*
 * assuming interleaved bitmap
 * assuming no compression
 */
static VOID body_to_bitmap(
	BitMapHeader* bmhd,
	UBYTE* body,
	BitMap* bm)
{
	/*
   * copy line by line of interleaved data
   * bpr, height and depth may differ
   */
	PLANEPTR dest = bm->Planes[0];
	UWORD dest_width = bm->BytesPerRow / bm->Depth;
	UWORD body_bpr = BYTES_PER_ROW(bmhd->bmh_Width, bmhd->bmh_Depth);
	UWORD copy_len = min(bmhd->bmh_Width >> 3, dest_width);
	UWORD height = min(bmhd->bmh_Height, bm->Rows);
	UWORD depth = min(bmhd->bmh_Depth, bm->Depth);

	int i, j;

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < depth; j++)
		{
			memcpy(
				&dest[(i * bm->BytesPerRow) + (j * dest_width)],
				&body[(i * body_bpr) + (j * (bmhd->bmh_Width >> 3))],
				copy_len
			);
		}
	}
}

static void decompress(
	const UBYTE* source,
	UBYTE* destination,
	const size_t compressed_size,
	const size_t decompressed_size)
{
	size_t src_idx = 0;
	size_t dest_idx = 0;

	while (src_idx < compressed_size && dest_idx < decompressed_size) {
		BYTE byte = source[src_idx++];

    if (byte >= 0) {
      int run_length = byte + 1;
			int i;
      for (i = 0; i < run_length && src_idx < compressed_size && dest_idx < decompressed_size; i++) {
      }
    } else if (byte != -128) {
    	int run_length = -byte;
			int i;
      for (i = 0; i < run_length && dest_idx < decompressed_size; i++) {
      }
    }
  }
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
