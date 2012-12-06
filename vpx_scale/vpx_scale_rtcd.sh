# Scaler functions
if [ "CONFIG_SPATIAL_RESAMPLING" != "yes" ]; then
    prototype void vp8_horizontal_line_5_4_scale "const unsigned char *source, unsigned int source_width, unsigned char *dest, unsigned int dest_width"
    prototype void vp8_vertical_band_5_4_scale "unsigned char *source, unsigned int src_pitch, unsigned char *dest, unsigned int dest_pitch, unsigned int dest_width"
    prototype void vp8_horizontal_line_5_3_scale "const unsigned char *source, unsigned int source_width, unsigned char *dest, unsigned int dest_width"
    prototype void vp8_vertical_band_5_3_scale "unsigned char *source, unsigned int src_pitch, unsigned char *dest, unsigned int dest_pitch, unsigned int dest_width"
    prototype void vp8_horizontal_line_2_1_scale "const unsigned char *source, unsigned int source_width, unsigned char *dest, unsigned int dest_width"
    prototype void vp8_vertical_band_2_1_scale "unsigned char *source, unsigned int src_pitch, unsigned char *dest, unsigned int dest_pitch, unsigned int dest_width"
    prototype void vp8_vertical_band_2_1_scale_i "unsigned char *source, unsigned int src_pitch, unsigned char *dest, unsigned int dest_pitch, unsigned int dest_width"
fi
