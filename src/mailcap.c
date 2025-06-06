/* This file reads the mailcap file, parses it and registers the
   preferences. There are two was to do this, either using PHB's
   mailcap library -- which is called from this file if PHILL is
   defined, or the handwritten parser found here */

/*
 **++
 **  FACILITY:  {@tbs@}
 **
 **  MODULE DESCRIPTION:
 **
 **      {@tbs@}
 **
 **  AUTHORS:
 **
 **      {@tbs@}
 **
 **  CREATION DATE:  {@tbs@}
 **
 **  DESIGN ISSUES:
 **
 **      {@tbs@}
 **
 **  [@optional module tags@]...
 **
 **  MODIFICATION HISTORY:
 **
 **      {@tbs@}...
 **-- */


/*
 **
 **  INCLUDE FILES
 **
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "www.h"


#ifdef PHILL
#include <pugwash_dir/pugwash.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif




#ifdef VMS
#include <unixio.h>
#else
#include <fcntl.h>
#endif


#ifdef PHILL

/* this block of include files needed to compile with mailcap */
#include <godel_dir/godel_support.h>
#include <godel_dir/type_support.h>
#include <Synthetic/mailcap_structure__structure.h>
#include <Synthetic/mailcap_interface.h>
#include <Synthetic/mailcap_parse_module.h>
#include <Synthetic/mailcap_errors.h>
#include <Synthetic/mime_internals__structure.h>

/* Following include files required only for this application */
#include <Synthetic/mailcap_args.h>
#include <Synthetic/mailcap.h>


mailcap_registry	*registry;
mailcap_query		*env;


ENTRY_POINT stream_parse_mime_header (char *filename,
				      mailcap_registry    **registry) {
    
    mailcap_context *env;
    FILE		*file_handle;
    char		buffer [1024];
    int			bytes = 1;
    
    BEGIN;
    
    file_handle = fopen (filename, "r");
    PRE (MAILCAP_COULD_NOT_READ, file_handle != NULL);
    
    MAILCAP_parse_open (&env);
    for (bytes = fread (buffer, 1, sizeof (buffer), file_handle);
	 bytes>0; bytes = fread (buffer, 1, sizeof (buffer), file_handle)) {
	
	MAILCAP_parse_block (env, &buffer, bytes);
    }
    fclose (file_handle);
    MAILCAP_parse_close (env, registry);
    
    END;
}

LOCAL_PROC put_block (void *context, void *block, int length) {
    BEGIN;
    
    puts (block);
    
    END;
}

#else


#define BUFSIZE 1000
extern int debug;
extern Context *context;

char *content_type, *view_command, *test_command;



#endif


void register_mailcaps()
{
    char *s, *mailcaps, *home, *file, *files;

    s = getenv("MAILCAPS");
    if (s)
	mailcaps = strdup(s);
    else {
	mailcaps = strdup(MAILCAPS);

	home = getenv("HOME");
	if (home) {
	    StrAllocCat(mailcaps, ":");
	    StrAllocCat(mailcaps, home);
	    StrAllocCat(mailcaps, "/.mailcap");
	}
    }

    if (MAILCAP_TRACE)
	fprintf(stderr,"register_mailcaps: %s\n",mailcaps);


    /* At this point we have a string that contains colon-separated
       mailcap files. Dissect the string, open the files and register
       their content */

    if ((file = str_tok(mailcaps,":",&files)))
	do {
	    register_mailcap(file);
	} while ((file = str_tok(NULL,":",&files)));

    Free(mailcaps);
}


void register_mailcap(char *mailcap)
{
  /*  janet 21/09/95: not used: char *home, *s, *files; */
    struct stat sbuf;
    double quality;

    if (MAILCAP_TRACE)
	fprintf(stderr,"register_mailcap: %s\n",mailcap);


#ifdef PHILL

    {
	VIEWER			viewer;
	EXPAND                  expand;
	char                    *string;
	int                    done = FALSE;
	BEGIN;
    
	viewer.filename = mailcap;
	viewer.uri = NULL;
	
	if (debug) 
	    printf ("Viewer %s\n", viewer.filename);
	STATUS = stream_parse_mime_header (viewer.filename, &registry);
	CHECK_STATUS;

	STATUS = MAILCAP_lookup_open (registry, NULL, &env);
	while (STATUS_OK && !done) {
	    mailcap_entry	*entry;
	    int test_result;
	    
	    STATUS = MAILCAP_lookup_next (env, &entry);
	    if (entry != NULL) {
	        char ct[100];

		sprintf(&ct,"%s/%s",entry->type.entry.content_major,entry->type.entry.content_minor);
		HTSetPresentation(conversions, ct, 
				  entry->type.entry.viewer, entry->type.entry.test, 
				  (entry->type.entry.quality ? (float)atof(entry->type.entry.quality) : (float)0.5),
				  0.9,
				  3.0, 0.0);

		if (debug)
		    printf ("Match %s %s %f\n", ct, entry->type.entry.viewer,entry->type.entry.test, 0.5);
	      }
	  }
      }

#else
    

    {
	/* HWL 23/9/94: a simple hand-coded mailcap parser */
	
        /* janet 21/07/95: not used:	FILE *fp; */
	int n, f;			/* janet 21/07/95: not used:  i, used=0 */
	char *buf, *line, *elem, *p, *field;
	

	if ((stat(mailcap, &sbuf) == -1) || sbuf.st_size <=0 ) /* do we really need another stat ? */
	    return;

	if ((f = open(mailcap, O_RDONLY)) == -1) {	
/*
	    if (MAILCAP_TRACE)
		HTAlert("Can't open mailcap file");
*/
	    return;
	}

	if ((buf = malloc (sbuf.st_size +1)) == NULL)
	    return;
	
	n = read(f, buf, sbuf.st_size);
	buf[n] = 0;

	p = buf;
	while ( (p = strstr(p,"\\\n")) ) {
	    p[0] = ' ';
	    p[1] = ' ';
	}
	
	p = str_tok(buf,"\n",&line);
	do {
	    content_type = view_command = test_command = NULL;
	    quality = 0.5;
	    
	    if (p[0] == '#') 
		continue;

	    if ( (p = str_tok(p,";",&elem)) == NULL)
		continue;
	    content_type = chop_str(p);
	    
	    if ( (p = str_tok(NULL,";",&elem)) == NULL)
		continue;
	    view_command = chop_str(p);
	    if (!strchr(view_command,'&')) {
		StrAllocCat(view_command, " &");
	    }
	    
	    /*	  add_content(content_type,view_command);*/

	    while ( (p = str_tok(NULL,";\n",&elem)) != NULL){
		field = chop_str(p);
		if (strlen(field) > 0) {
		    char *pp;
		    
		    if (strncmp("test",field,4) == 0) {
			pp = field + 4;
			while (isspace(*pp) || *pp == '=')
			    pp++;
			test_command = strdup(pp);
		    }
		    else if (strncmp("quality",field,7) == 0) {
			pp = field + 7;
			while (isspace(*pp) || *pp == '=')
			    pp++;
			quality = (double)atof(pp);
		    }
		}
		Free (field);
	    }


	    if (MAILCAP_TRACE) {
/*
	        fprintf(stderr,"register_mailcap, HTSetPresentation: %s \"%s\" %f", content_type, view_command, (quality >= 0.0 ? quality : 0.5));
*/
	        if (test_command) { 
		    fprintf(stderr,"-%s\n",test_command); 
		} else { 
		    fprintf(stderr,"\n"); 
		}
	    }

	    quality = (quality >= 0.0 ? quality : 0.5);

	    if (HTCheckPresentation(context->conversions, content_type, quality))
	    {
		HTConversion_add(context->conversions, content_type, 
				  view_command, (HTConverter *)test_command, 
				  quality, 0.0, 0.0);
		HTPresentation_add(context->conversions, content_type, 
				  view_command, (HTConverter *)test_command, 
				  quality, 0.0, 0.0);
	    }
#if 0 
was...
	 HTConversion_add(context->conversions, content_type, 
				  view_command, (HTConverter *)test_command, 
				  quality, 0.0, 0.0);
#endif
	    Free(content_type);
	    Free(view_command);
	    Free(test_command);
	} while ( (p = str_tok(NULL,"\n",&line)) ); 
	close(f);
	Free(buf);

    }
#endif /* PHILL */

}




