#ifndef IO_H
#define IO_H

#include "datatypes.h"


BOOL load_picture(CONST_STRPTR path, PictureData*);
BOOL save_picture(CONST_STRPTR path, PictureData*);

#endif
