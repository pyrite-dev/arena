/*                                                                            Icon Management
                                     ICON MANAGEMENT
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module is implemented by HTIcons.c, and it is a part of the W3C Reference Library.
   
 */
#ifndef HTICONS_H
#define HTICONS_H

#include "HTFormat.h"
/*

Icons

   Icons are bound to MIME content-types and encoding.  These functions bind icon URLs to
   given content-type or encoding templates.  Templates containing a slash are taken to be
   content-type templates, other are encoding templates.
   
   
   
Controlling globals

  SHOW BRACKETS AROUND ALTERNATIVE TEXT
  
   By default alternative text is bracketed by square brackets (the ALT tag to IMG
   element).  Setting the global HTDirShowBrackets to false will turn this feature off.
   
 */
typedef struct _HTIconNode {
    char *      icon_url;
    char *      icon_alt;
    char *      type_templ;
} HTIconNode;

/*
 * The list element definition to bind a CGI to a filetyp for special
 * presentation like looking in an archiv (AddHref /cgi-bin/unarch? .zip .tar)
 */
typedef struct _HTHrefNode {
    char *      href_url;
    char *      type_templ;
} HTHrefNode;
/*

File Mode

   This is a simplified file mode enumeration that can is used in directory listings.
   
 */
typedef  enum _HTFileMode {
    HT_IS_FILE,                         /* Normal file */
    HT_IS_DIR,                          /* Directory */
    HT_IS_BLANK,                        /* Blank Icon */
    HT_IS_PARENT                        /* Parent Directory */
} HTFileMode;
/*

Public functions

   All of these functions take an absolute URL and alternate text to use.
   
 */
/* Generates the alt-tag */
extern char * HTIcon_alt_string (char * alt,
                                        BOOL   brackets);

/*
 * General icon binding.  Use this icon if content-type or encoding
 * matches template.
 */
extern void HTAddIcon (char *   url,
                              char *    alt,
                              char *    type_templ);


/*
 * Called from HTConfig.c to build the list of all the AddHref's
 */
extern void HTAddHref (char *    url,
                              char *    type_templ);


/*
 * Icon for which no other icon can be used.
 */
extern void HTAddUnknownIcon (char * url,
                                     char * alt);

/*
 * Invisible icon for the listing header field to make it aligned
 * with the rest of the listing (this doesn't have to be blank).
 */
extern void HTAddBlankIcon (char * url,
                                   char * alt);

/*
 * Icon to use for parent directory.
 */
extern void HTAddParentIcon (char * url,
                                    char * alt);

/*
 * Icon to use for a directory.
 */
extern void HTAddDirIcon (char * url,
                                 char * alt);

/*                                                               HTGetIcon()
** returns the icon corresponding to content_type or content_encoding.
*/
extern HTIconNode * HTGetIcon (HTFileMode       mode,
                                      HTFormat          content_type,
                                      HTEncoding        content_encoding);

/*
 * returns the HrefNode to get the URL for presentation of a file (indexing)
 */
extern HTHrefNode * HTGetHref ( char *  filename);


/*

    A Predifined Set of Icons
    
   The function HTStdIconInit(url_prefix) can be used to initialize a standard icon set:
   
       blank.xbm for the blank icon
      
       directory.xbm for directory icon
      
       back.xbm for parent directory
      
       unknown.xbm for unknown icon
      
       binary.xbm for binary files
      
       text.xbm for ascii files
      
       image.xbm for image files
      
       movie.xbm for video files
      
       sound.xbm for audio files
      
       tar.xbm for tar and gtar files
      
       compressed.xbm for compressed and gzipped files
      
 */
extern void HTStdIconInit (CONST char * url_prefix);
/*

 */
#endif /* HTICONS */
/*

   end */
