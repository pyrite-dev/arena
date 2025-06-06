#ifndef _ARENA_PNG_H_
#define _ARENA_PNG_H_

unsigned char *ReadPNGImage_1_2_4(png_struct *, png_info *, png_byte *); 
unsigned char *ReadPNGImage_24(png_struct *, png_info *, png_byte *); 
unsigned char *ReadPNGImage_16(png_struct *, png_info *, png_byte *); 
unsigned char *ReadPNGImage(png_struct *, png_info *, png_byte *); 
unsigned char *LoadPNGImage(Image *, Block *, unsigned int);
unsigned char *LoadProgressivePNGImage(Image *, Block *, unsigned int, unsigned long);

#endif /* _ARENA_PNG_H_ */
