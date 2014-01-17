#include "y4menc.h"

void y4m_write_file_header(FILE *file,
                           int width, int height, int num, int den,
                           vpx_img_fmt_t fmt) {
  const char *color = fmt == VPX_IMG_FMT_444A ? "C444alpha\n" :
                      fmt == VPX_IMG_FMT_I444 ? "C444\n" :
                      fmt == VPX_IMG_FMT_I422 ? "C422\n" : "C420jpeg\n";

  // Note: We can't output an aspect ratio here because IVF doesn't
  // store one, and neither does VP8.
  // That will have to wait until these tools support WebM natively.*/
  fprintf(file, "YUV4MPEG2 W%u H%u F%u:%u I%c %s",
          width, height, num, den, 'p', color);
}

void y4m_write_frame_header(FILE *file) {
  fprintf(file, "FRAME\n");
}
