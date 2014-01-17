#ifndef Y4MENC_H_
#define Y4MENC_H_

#include <stdio.h>

#include "vpx/vpx_decoder.h"

void y4m_write_file_header(FILE *file,
                           int width, int height, int num, int den,
                           vpx_img_fmt_t fmt);

void y4m_write_frame_header(FILE *file);


#endif  // Y4MENC_H_
