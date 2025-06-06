#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include "www.h"

void *GetPointer(unsigned char **p)
{
#if defined POINTER_IS_64BIT
    unsigned long c1, c2; /* must hold a 64bit Pointer wm 19.Jan.95 */
#else
    unsigned int c1, c2;
#endif /* POINTER_IS_64BIT */
    long str;
    unsigned char *pp;

    pp=*p;

    c1 = *pp++; c2 = *pp++; str = (c1 | c2<<8);
    c1 = *pp++; c2 = *pp++; str |= (c1 | c2<<8) << 16;
#if defined POINTER_IS_64BIT
    c1 = *pp++; c2 = *pp++; str |= (c1 | c2<<8) << 32;
    c1 = *pp++; c2 = *pp++; str |= (c1 | c2<<8) << 48;
#endif /* POINTER_IS_64BIT */
    *p=pp;
    return (void *)str;
}

void PutPointer(unsigned char **p, void *ptr)
{
     unsigned int ptr_16byte;
     unsigned char *pp;

     pp=*p; 
     ptr_16byte = (unsigned int)((long)ptr & 0xFFFF);
     *pp++ = (unsigned char) ( ptr_16byte & 0xFF );
     *pp++ = (unsigned char) ( (ptr_16byte >> 8) & 0xFF );
     ptr_16byte = (unsigned int)( ((long)ptr >> 16) & 0xFFFF);
     *pp++ = (unsigned char) ( ptr_16byte & 0xFF );
     *pp++ = (unsigned char) ( (ptr_16byte >> 8) & 0xFF );
#if defined POINTER_IS_64BIT
     ptr_16byte = (unsigned int)( ((long)ptr >> 32) & 0xFFFF);
     *pp++ = (unsigned char) ( ptr_16byte & 0xFF );
     *pp++ = (unsigned char) ( (ptr_16byte >> 8) & 0xFF );
     ptr_16byte = (unsigned int)( ((long)ptr >> 48) & 0xFFFF);
     *pp++ = (unsigned char) ( ptr_16byte & 0xFF );
     *pp++ = (unsigned char) ( (ptr_16byte >> 8) & 0xFF );
#endif /* POINTER_IS_64BIT */
     *p=pp; 
}
