#include "www.h"


#include "WWWLib.h"			      /* Global Library Include file */
#include "WWWNews.h"				       /* News access module */
#include "WWWHTTP.h"				       /* HTTP access module */

#include "HTBound.h"
#include "HTGuess.h"
#include "HTFile.h"
#include "HTFTP.h"
#include "HTGopher.h"
#include "HTXParse.h"
#include "HTReqMan.h"
#include "HTParse.h"
#include "HTCache.h"
#include "HTAlert.h"
#include "HTProxy.h"
#include "HTError.h"
#include "HTDialog.h"
#include "HTMIME.h"
#include "HTMethod.h"
#include "HTFWrite.h"
#include "HText.h"
#include "HTDNS.h"
#include "HTHome.h"
#include "HTAnchor.h"
#include "HTAncMan.h"

extern Display *display;
extern Doc *CurrentDoc;
extern int debug;
extern char *CacheRoot;
extern int document;
extern int NoMailcap;
extern char *buffer;

extern int initialised;
extern Context *context;
extern Doc *CurrentDoc;
extern int debug;
extern char *CacheRoot;
extern int NoMailcap;
extern int AbortFlag;
extern int library_trace;
extern int initialised;

HTAtom *text_atom;
HTAtom *html_atom;
HTAtom *html3_atom;
HTAtom *html_level3_atom;
HTAtom *gif_atom;
HTAtom *jpeg_atom;
HTAtom *png_atom;
HTAtom *png_exp_atom;
HTAtom *xpm_atom;
HTAtom *xbm_atom;

char *HTAppName = BANNER;
char *HTAppVersion = VERSION;

extern int sbar_width;
extern unsigned int win_width, win_height;
extern int ToolBarHeight;
extern int statusHeight;
extern int buf_width;
extern long buf_height;
extern int PixelIndent;

PUBLIC BOOL AHTProgress (HTRequest * request, HTAlertOpcode op,
			int msgnum, CONST char * dfault, void * input,
			HTAlertPar * reply)
{

    switch (op) {
      case HT_PROG_DNS:
	Announce("Looking up %s\n", input ? (char *) input : "");
	break;

      case HT_PROG_CONNECT:
#if 0	
        {
	    char *s = HTAnchor_physical(HTRequest_anchor(request));
	    if (s)
	        Announce("Contacting host %s\n", s);
	    break;
	}
#else	
	Announce("Contacting host %s\n", input ? (char *) input : "");
	break;
#endif
      case HT_PROG_ACCEPT:
	Announce("Waiting for connection...\n");
	break;

      case HT_PROG_READ:
	{
	    long cl = HTAnchor_length(HTRequest_anchor(request));
	    if (cl > 0) {
		long b_read = HTRequest_bytesRead(request);
		double pro = (double) b_read/cl*100;
		char buf[10];
		HTNumToStr((unsigned long) cl, buf, 10);
		Announce("Read (%d%% of %s)\n", (int) pro, buf);
	    } else
		Announce("Reading...\n");
	}
	break;

      case HT_PROG_WRITE:
	if (HTRequest_isPostWeb(request)) {
	    HTParentAnchor *anchor=HTRequest_anchor(HTRequest_source(request));
	    long cl = HTAnchor_length(anchor);
	    if (cl > 0) {
		long b_write = HTRequest_bytesWritten(request);
		double pro = (double) b_write/cl*100;
		char buf[10];
		HTNumToStr((unsigned long) cl, buf, 10);
		Announce("Written (%d%% of %s)\n", (int) pro, buf);
	    } else
		Announce("Writing...\n");
	}
	break;

      case HT_PROG_DONE:
	Announce("Finished\n");
	break;

      case HT_PROG_WAIT:
	Announce("Waiting for free socket...\n");
	break;

      default:
	Announce("UNKNOWN PROGRESS STATE\n");
	break;
    }
    return YES;
}


int redirection_handler (HTRequest * request, int status)
{
    Context * context = (Context *) HTRequest_context(request);
    HTMethod method = HTRequest_method(request);
    HTAnchor * new_anchor = HTRequest_redirection(request);

    History *h;
    HTList *l; /* 18-Mar-96 Thomas Quinot <thomas@cuivre.fdn.fr> */
    Doc *doc = (Doc *) context;

    if (!(context))
    {
	fprintf (stderr, "redirection_handler: NO DOC FOUND IN CONTEXT ???\n");
	return HT_ERROR;
    }
    
    l = context->history;
    /* update history in case of redirection */
    
    while ((h = (History *)HTList_nextObject(l))) {
        if (h->anchor->parent == doc->anchor) {
	    h->anchor = (request->childAnchor ? (HTAnchor *)request->childAnchor : (HTAnchor *)request->anchor);
	    doc->anchor = request->anchor->parent;
	}
    }

    /* Make sure we do a reload from cache */
    HTRequest_setReloadMode(request, HT_FORCE_RELOAD);

    /* If destination specified then bind source anchor with new destination */
#if 0
    if (HTMethod_hasEntity(method)) {
        HTAnchor_removeAllLinks((HTAnchor *)context->source);
        HTAnchor_link((HTAnchor *) context->source, new_anchor, NULL, method);
    }
#endif
    /* Log current request */
/*    if (HTLog_isOpen()) HTLog_add(request, status);*/

HTLoadAnchor(new_anchor, request);
#if 0
    /* Start new request */
    if (HTRequest_retry(request)) {
        if (HTMethod_hasEntity(method))                    /* PUT, POST etc. */
        {
	/*       HTCopyAnchor((HTAnchor *) context->source, request); */
	}
        else                                       /* GET, HEAD, DELETE etc. */
            HTLoadAnchor(new_anchor, request);
    } else {
    /* show error */
    }
#endif
    return HT_ERROR;      /* Make sure this is the last callback in the list */
}


Doc *DocNew()
{
    Doc *c;

    c = ((Doc *) calloc (sizeof(Doc), 1));
    c->state = DOC_NOTREGISTERED;
    return c;
}

#define SEC_TIMEOUT 1
#define USEC_TIMEOUT 0


/* utility functions that should go elsewhere */

void DisplayUrl()
{
    if (CurrentDoc && CurrentDoc->url) {
	if (CurrentDoc->tag)
	    Announce("%s#%s",CurrentDoc->url,CurrentDoc->tag);
	else
	    Announce("%s",CurrentDoc->url);
    }
}


BOOL DocImage(Doc *d)
{
    HTAtom *a=NULL;

    if (!d || !d->anchor)
	return False;
    
    a = HTAnchor_format(d->anchor);

    return(
	   (a == gif_atom) || 
#ifdef JPEG
	   (a == jpeg_atom) || 
#endif
#ifdef PNG
	   (a == png_atom) || 
	   (a == png_exp_atom) || 
#endif
	   (a == xpm_atom) ||
	   (a == xbm_atom)
	   );
}

BOOL DocRawText(Doc *d)
{
    HTAtom *a=NULL;

    if (!d || !d->anchor)
	return False;

    a = HTAnchor_format(d->anchor);

    if ((d->show_raw) || (a == text_atom))
	return TRUE;
    else
	return FALSE;
}


BOOL DocHTML(Doc *d)
{
    HTAtom *a;

    if (!d || !d->anchor)
	return True;		/* this being the default type */

    a = HTAnchor_format(d->anchor);

    return((a == html3_atom) || (a == html_level3_atom) || (a == html_atom));
}


/* Routines to free documet structures */

void DocFree(Doc *doc, Bool cut_anchor)
{
    /* normally, we don't want to free a document if its pending. In
       other cases, the stat flag should be manipulated first */

    if (!doc || doc->nofree || doc->state == DOC_PENDING)
	return;

    if (BRIDGE_TRACE)
	if (doc->url != NULL)
	    fprintf(stderr,"DocFree: freeing %s\n", doc->url);
	else
	    fprintf(stderr,"DocFree\n");
	
/*    Free (doc->href);*/
/*    Free (doc->url); */
/*    Free (doc->request); */ /* problem: if we free this while the doc is still being fetched.. check state.. */
    HTList_addObject(context->memory_leaks, (void *)doc->request);

    Free (doc->content_buffer);
    Free (doc->paint_stream);
/*
 * hum... we must remove all pixmaps hidden in the paint stream before freeing it 
 */

    /* if inline, we have to go through the list of host docs,  .. doing what? nothing! */

    HTList_delete (doc->host_anchors);
    HTList_delete (doc->inline_anchors);
    HTList_free (doc->bad_flags);

    if (doc->image) {
	if (BRIDGE_TRACE)
	    fprintf(stderr,"  freeing pixmap: %ld\n", doc->image->pixmap);
	XFreePixmap (display, doc->image->pixmap);
	Free (doc->image);
    }

    /* should we remove it from this list? Probably not..*/
/*    HTList_removeObject(context->registered_anchors, (HyperDoc *) doc);*/

    if (cut_anchor)
	doc->anchor->document = NULL;

    if (doc->style)
	FreeStyleSheet(doc->style);

    Free(doc);
}


void DocsFree(Doc *doc)
{
    HTList *l;
    Doc *d;
    HTParentAnchor *a;

    if (!doc || doc->nofree)
	return;

    if (doc->url != NULL) {
	if (BRIDGE_TRACE)
	    fprintf(stderr,"DocsFree: freeing %s\n", doc->url);
    }
    else {
	if (BRIDGE_TRACE)
	    fprintf(stderr,"DocsFree, no url given\n");
    }

    /* if main doc, we have to go through the inline, remove ourselves
       from the list of hosts, delete the document if we're the last
       main_doc pointing to it */

    l = doc->inline_anchors;
    while ((a = (HTParentAnchor *)HTList_nextObject(l))) {
	d = (Doc *)a->document;
	if (d && doc->anchor) {
	    HTList_removeObject(d->host_anchors, doc->anchor);
	    if (HTList_count(d->host_anchors) == 0) {
		DocFree(d, TRUE);
		a->document = NULL;
	    }
	}
    }
    DocFree(doc, TRUE);
    if (BRIDGE_TRACE && VERBOSE_TRACE)
	libSane();
}


/* libsane is a debuging routine that checks the state of document pointers */

void libSane()
{
}



/* HTCheckPresentation is used to decide if a content type should be
   registered or not. If the content type is not registered before,
   return TRUE to have it registered. If the content type has been
   registered, check the quality associated with it. */


BOOL HTCheckPresentation(HTList * conversions, char * content_type, double quality)
{
    HTList *l = conversions;
    HTPresentation *p;

    while ((p = (HTPresentation *)HTList_nextObject(l))) {
	if (strcmp(content_type, p->rep->name) == 0) {
	    if (p->quality < quality)
		return TRUE;
	    else {
		if (MAILCAP_TRACE)
		    fprintf(stderr,"HTCheckPresentation returning FALSE for %s\n",content_type);
		return FALSE;
	    }
	}
    }
    return TRUE;
}




/* ------------------------------------------------------------------------- */
/*				REQUEST FUNCTIONS			     */
/* ------------------------------------------------------------------------- */

/*
**  This function creates a new request structure and adds it to the global
**  list of active threads
*/
HTRequest *ReqNew(BOOL Interactive)
{
    HTRequest *newreq = HTRequest_new(); 	     /* Set up a new request */
    HTRequest_setPreemptive(newreq, NO);
/*    HTRequest_setPreemtive(newreq, NO);*/
    HTRequest_addRqHd(newreq, HT_C_HOST);
    HTRequest_addRqHd(newreq, HT_C_REFERER);
    HTRequest_setGnHd(newreq, HTRequest_gnHd(newreq) & ~HT_G_CONNECTION);

    HTRequest_setOutputStream(newreq, HTXParse(newreq, NULL, NULL, NULL, NULL));
    Free(newreq->conversions); 	/* janet 28/07/95: added this to free memory */
    newreq->conversions = context->conversions;

    HTList_addObject(context->reqs, (void *) newreq);
    return newreq;
}


/*
**  This function deletes a request structure and takes it out of the list
**  of active threads.
*/
void ReqDel(HTRequest *oldreq)
{
    if (oldreq) {
	if (context->reqs)
	    HTList_removeObject(context->reqs, (void *) oldreq);
	HTRequest_delete(oldreq);
    }
}


/*
**  This function deletes the whole list of active threads.
*/
void ReqDelAll()
{
    HTList *l;

    if (context == NULL)
	return;
    l = context->reqs;
    if (l) {
	HTList *cur = l;
	HTRequest* pres;
	while ((pres = (HTRequest *) HTList_nextObject(cur)) != NULL) {
	    HTRequest_delete(pres);
	}
	HTList_delete(l);
	context->reqs = NULL;
    }
}



int EventHandler(SOCKET s, HTRequest * req, SockOps ops)
{
    /* janet 21/07/95: not used:    HTParentAnchor *anchor; */
    /* janet 21/07/95: not used:    Doc *doc; */

    GuiEvents(0, NULL, 0);
    GuiEvents(0, NULL, 0);
    GuiEvents(0, NULL, 0);

    if (req && req->anchor) {
/*	anchor = libFindAnchorInList(*req);*/
	if (BRIDGE_TRACE && VERBOSE_TRACE)
	    fprintf(stderr,"libCallback, %s\n", req->anchor->parent->address);
    }

    if (AbortFlag) {	/* we'll have to abort this session */
	if (BRIDGE_TRACE)
	    fprintf(stderr,"libcallback: aborting req\n");
	AbortFlag = 0;
	HideBusy();
	DisplayUrl();

	HTNet_killAll();  /* interrupt everything !! */
	return HT_OK;
    }
    else {
	if (BRIDGE_TRACE && VERBOSE_TRACE)
	    fprintf(stderr,"libcallback EVENT_OK\n");
	/*	return EVENT_OK; */
	return HT_OK;
    }
}






/*
 * this function is the arena frontend to HTLoad
 *
 * href
 * hreflen
 * who		currently unused
 * main_doc	is this a main document or an inline referenced from a host_doc
 * host_doc	if inline, this points to host document
 */

HTAnchor *libExpandHref(char *href, size_t hreflen) /* wm 18.Jan.95 */
{
    /*  janet 21/07/95: not used:   int c; */
    char *ref;
    char *hrefdup = NULL;
    char *relname;
    HTChildAnchor *child_anchor;
    HTParentAnchor *parent_anchor;
    HTAnchor *anchor;
    
    hrefdup = www_strndup(href,hreflen);

    if (strncmp(href,"mailto:",7) == 0) { 
	Announce("mailto not supported: %s",hrefdup);
	Free(hrefdup);
	return NULL;
    }

    if(CurrentDoc && CurrentDoc->base) {
        ref = HTParse(hrefdup, CurrentDoc->base, PARSE_ALL);
    } else if (CurrentDoc && CurrentDoc->url) {
        ref = HTParse(hrefdup, CurrentDoc->url, PARSE_ALL);
    } else {
      /* janet 2/8/95: HTFindRel.. is not freed when used as such: */
      /*ref = HTParse(hrefdup, (char *)HTFindRelatedName(), PARSE_ALL);  */
        relname  = HTFindRelatedName(); 		/* janet 2/8/95: added  */
        ref = HTParse(hrefdup, relname, PARSE_ALL);	/* janet 2/8/95: changed  */
	Free(relname);					/* janet 2/8/95: added  */
    }	
  

    if (BRIDGE_TRACE)
	fprintf(stderr,"---->libExpandHref expanding %s into  %s\n", hrefdup, ref);

    anchor = (HTAnchor *) HTAnchor_findAddress(ref); /* we have to free anchor->address */
    child_anchor = (HTChildAnchor *) anchor;
    parent_anchor = child_anchor->parent;

    Free(ref);
    
    if (parent_anchor->document) {
	if (BRIDGE_TRACE)
	    fprintf(stderr,"libExpandHref: document structure for %s already in memory\n",hrefdup);
	if (BRIDGE_TRACE && VERBOSE_TRACE)
	    libSane();
    }

    Free(hrefdup);    
    return anchor;
}


/*

libLoadAnchor creates a document structure and starts the process of
filling it. 

*/

Doc * libLoadAnchor(HTAnchor *anchor, Doc *host_doc, BOOL main_doc, BOOL nofree, BOOL record, BOOL reload, BOOL block, BOOL for_real)
{
    Doc *doc = NULL;
    int status;
    HTRequest *request;
    BOOL forreal;

    History *h;
    HTList *l;

/*    char *adr = anchor->parent->address; */


/*    Announce("Fetching %s", adr);*/

    if (main_doc) {
	ShowBusy();
    }

    forreal = for_real;
    request = ReqNew(YES);
    
    if (reload)
	request->reload = HT_CACHE_REFRESH; /* see HTReq.h */
    else
    {
        /*History *h;
	HTList * */l = context->history;
	
	while ((h = (History *)HTList_nextObject(l))) {
	    if (h->anchor->parent == anchor) {
		doc = h->anchor->parent->document;
		if(doc)
		    return doc;
		else
		{
		    doc = DocNew();
		    doc->state = DOC_LOADED;
		    doc->request = request;
		    doc->anchor = anchor->parent;
		    anchor->parent->document = (void *) doc;
		    doc->main_doc = main_doc;
		    doc->nofree = nofree;
		    doc->url = anchor->parent->address;
		    
		    HTRequest_setContext(request, (void *) doc);
		}
	    }
	}
    }

    /* Set up doc before calling HTLoadAnchor. If a file is in the
     * cache the terminate handler will be called. If the doc is not
     * set up correctly the terminate handler can not find it and
     * will not process the doc. Herman ten Brugge <herman@htbrug.hobby.nl>
     * 10-Mar-96
     */
    doc = DocNew();
    doc->state = DOC_PENDING;
    doc->request = request;
    doc->anchor = anchor->parent;
    anchor->parent->document = (void *) doc;
    doc->main_doc = main_doc;
    doc->nofree = nofree;
    doc->url = anchor->parent->address;

    HTRequest_setContext(request, (void *) doc);


    if (host_doc && host_doc->anchor)
	request->parentAnchor = HTAnchor_parent((HTAnchor *) host_doc->anchor);

    if (forreal) {
	status = HTLoadAnchor((HTAnchor*) anchor, request);

	if (status == NO) {
	    Free(doc);
	    return NULL;
	}
    }
#if 0
    doc = DocNew();
    doc->request = request;
    doc->anchor = anchor->parent;
    anchor->parent->document = (void *) doc;
    HTRequest_setContext(request, (void *) doc);
#endif
    if (main_doc && record) {
        HistoryRecord(anchor);
    }

        
    /* register the document */ 
    
    if (!main_doc && host_doc) {

	/* first, put the inline in the host_doc's list of inlines */

	if (!host_doc->inline_anchors)
	    host_doc->inline_anchors = HTList_new();
	HTList_addObject(host_doc->inline_anchors, (void *) doc->anchor);
	
	/* then, put the host_doc in the inline's list of host_docs */

	if (!doc->host_anchors)
	    doc->host_anchors = HTList_new();
	HTList_addObject(doc->host_anchors, (void *) host_doc->anchor);
    }
#if 0    
    doc->main_doc = main_doc;
    doc->nofree = nofree;
    doc->url = anchor->parent->address;
#endif
    if (main_doc) {
	if (BRIDGE_TRACE) {
	    fprintf(stderr,"---->libLoadAnchor registering maindoc: %s\n", doc->url);
	}
    }

/*    doc->state = DOC_PENDING; */
    return doc;
}
#if 0
int PostCallback (HTRequest * request, HTStream * target)
{
 if (!request || !request->source_anchor || !target) return HT_ERROR;
    {
	int status;
	HTParentAnchor * source = request->source_anchor;
	char * document = (char *) HTAnchor_document(request->source_anchor);
	int len = HTAnchor_length(source);

	status = (*target->isa->put_block)(target, document, len);
	if (status == HT_OK)
	    return (*target->isa->flush)(target);
	if (status == HT_WOULD_BLOCK) {
	    if (PROT_TRACE)TTYPrint(TDEST,"POST Anchor. Target WOULD BLOCK\n");
	    return HT_WOULD_BLOCK;
	} else if (status == HT_PAUSE) {
	    if (PROT_TRACE) TTYPrint(TDEST,"POST Anchor. Target PAUSED\n");
	    return HT_PAUSE;
	} else if (status > 0) {	      /* Stream specific return code */
	    if (PROT_TRACE)
		Announce("POST Anchor. Target returns %d\n", status);
	    return status;
	} else {				     /* We have a real error */
	    if (PROT_TRACE) Announce("POST Anchor. Target ERROR\n");
	    return status;
	}
    }
}
#endif

Doc *libPostDocument(char *href, size_t hreflen, char *post, size_t postlen)
{
    HTAnchor *anchor = libExpandHref(href, hreflen);
    HTAnchor *src;
    Doc *doc = NULL;
    char *name;
    HTRequest *request;

    if (!anchor)
	return NULL;
    
    name = (char *)malloc(strlen(href)+16);
    sprintf(name, "internal:post/%s",href);

    Announce("Posting %s", anchor->parent->address);
    ShowBusy();

    request = HTRequest_new();
    HTRequest_setPreemptive(request, NO);
    HTRequest_addRqHd(request, HT_C_HOST);
    HTRequest_setOutputStream(request, HTXParse(request, NULL, NULL, NULL, NULL));
    Free(request->conversions); 	
    request->conversions = context->conversions;
    HTRequest_setMethod(request, METHOD_POST);
    
    src = HTAnchor_findAddress(name);
    HTAnchor_setDocument((HTParentAnchor *)src, post);
    HTAnchor_setFormat((HTParentAnchor *)src, HTAtom_for("application/x-www-form-urlencoded"));
    HTAnchor_setLength((HTParentAnchor *)src, postlen);

    HTAnchor_link(src, anchor, NULL, METHOD_POST);	
/*    HTUploadAnchor((HTAnchor *) src, request,
 *		   HTUpload_callback);
 * Herman ten Brugge <herman@htbrug.hobby.nl>
 */  
    doc = DocNew();
    doc->request = request;
    doc->state = DOC_PENDING;
    doc->anchor = anchor->parent;
    anchor->parent->document = (void *) doc;
    doc->main_doc = TRUE;
    doc->nofree = TRUE;
    doc->url = anchor->parent->address;
    HTRequest_setContext(request, (void *) doc);
    HTUploadAnchor((HTAnchor *) src, request,
		   HTUpload_callback);
    HistoryRecord(anchor);

    /* register the document */ 
    
    if (BRIDGE_TRACE) {
	    fprintf(stderr,"---->libPostAnchor registering maindoc: %s\n", doc->url);
    }    
    return doc;
}

HTAnchor *libGetDocument(char *href, size_t hreflen, Doc *host_doc, BOOL main_doc, BOOL nofree, BOOL record, BOOL reload, BOOL block)
{
    HTAnchor *anchor = libExpandHref(href, hreflen);

    if (!anchor)
	return NULL;

    if (reload) {
	CurrentDoc->pending_reload = TRUE;
    } 

    if (anchor->parent->document) {
	Doc *d = (Doc *)anchor->parent->document;

	switch (d->state) {
	  case DOC_NOTREGISTERED:
	  case DOC_ABORTED:
	  case DOC_REJECTED:
	    libLoadAnchor(anchor, host_doc, main_doc, nofree, record, reload, block, TRUE);
	    break;
	  case DOC_PENDING:
	  case DOC_EXTERNAL:
	    break;
	  case DOC_LOADED:
	  case DOC_PROCESSED:
	      if(reload)
		  libLoadAnchor(anchor, host_doc, main_doc, nofree, record, reload, block, TRUE);
	      else
	      {
		  CurrentDoc = (Doc *) d;
		  /* CurrentDoc->tag = h->tag; */
		  CurrentDoc->show_raw = FALSE;
		  NewBuffer(CurrentDoc);
		  DeltaHTMLPosition(context->current_history->y);
		  DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
		  SetScrollBarHPosition(PixelIndent, buf_width);
		  SetScrollBarVPosition(context->current_history->y, buf_height);
		  DisplayScrollBar();
		  DisplayUrl();
	      }
	      break;
	}
    } else
        libLoadAnchor(anchor, host_doc, main_doc, nofree, record, reload, block, TRUE);

    return anchor;
}



Doc *GetInline(char *href, int hreflen, BOOL reload)
{
    HTAnchor *a;
    Doc *doc = NULL;

    /* check for null name */

    if (href == NULL || hreflen == 0)
    {
        Warn("Missing or bad inline name");
        return NULL;
    }

    /* first, lets expand the href into an anchor */

    a = libExpandHref(href, hreflen);

    /* check if document is already in place */

    if (a->parent->document && !reload) {
	doc = (Doc *)a->parent->document;
    }
    else {

	/* no, the document does not seem to be in place, lets register it.. */

        /* first, clear any reference to previously loaded document.. */
      	a->parent->document = NULL;  


	if (BRIDGE_TRACE) {
	    char *s = www_strndup(href, hreflen);
	    fprintf(stderr,"GetInline registering %s\n",s);
	    Free(s);
	}
	
	libLoadAnchor(a, CurrentDoc, FALSE, FALSE, FALSE, reload, FALSE, TRUE); /* howcome 1/8/95: record should be false, no? */

	/* if image or style is still not there.. */

	if (!a->parent->document) {
	  /* */
	    return NULL;
	} else
	    doc = (Doc *) a->parent->document;
    }

    if (doc) {

	/* the inline doc has been registered, it can be loading, loaded, processed*/

	switch (doc->state) {

	  case DOC_PENDING:
	    if (BRIDGE_TRACE) {
		char *s = www_strndup(href, hreflen);
		fprintf(stderr,"doc has been registered, but not loaded -> returning default%s\n",s);
		Free(s);
	    }
	    return NULL;
	    break;

	  case DOC_PROCESSED:
	    if (BRIDGE_TRACE)
		fprintf(stderr,"GetImage: doc processed returning image %s\n",doc->url);
	    return doc;
	    break;

	  case DOC_LOADED:
	    return doc;
	}
    }
    return NULL;
}




#if 0

/*
** Check if document is already loaded. As the application handles the
** memory cache, we call the application to ask. Also check if it has
** expired in which case we reload it (either from disk cache or remotely)
*/
int libMemoryCache (HTRequest * request, HTExpiresMode mode,
			  char * notification)
{
    Doc *doc;
    if (!request)
	return HT_ERROR;
    if ((doc = (Doc *) HTAnchor_document(request->anchor))) {
	if (request->reload != HT_MEM_REFRESH) {
	    if (CACHE_TRACE)
		fprintf(TDEST,"HTMemCache.. Document already in memory\n");
	    if (mode != HT_EXPIRES_IGNORE) {
		if (!HTCache_isValid(request->anchor)) {
		    if (mode == HT_EXPIRES_NOTIFY)
			HTAlert(request, notification);
		    else {
			if (CACHE_TRACE)
			    fprintf(TDEST,
				    "HTMemCache.. Expired - autoreload\n");
			request->RequestMask |= HT_IMS;
#ifndef HT_SHARED_DISK_CACHE
			request->reload = HT_CACHE_REFRESH;
#endif
			return HT_ERROR; /* Must go get it */
		    }
		}
	    }


	    /*
	     ** If we can use object then select it and return
	     ** This should be a callback to the app mem cache handler
	     */
	    if (request->childAnchor)
		HText_selectAnchor(text, request->childAnchor);
	    else
		HText_select(text);


	    return HT_LOADED;
	} else {		/* If refresh version in memory */
	    request->RequestMask |= HT_IMS;
	}
    } else {			      /* Don't reuse any old metainformation */
	HTAnchor_clearHeader(request->anchor);
    }
    return HT_ERROR;
}

#endif

/* libEntry is called quite early in the process */

void libEntry(int c)
{
    WWW_TraceFlag = library_trace; /* turn on debugging in Library */

    HTLibInit(BANNER, VERSION);

    HTNet_setMaxSocket(20);

/*
    HTMemoryCache_register(libMemoryCache);
*/
    HTProxy_getEnvVar();	/* read old environment variables */

    HTProtocol_add("http", NO, HTLoadHTTP, NULL);
    HTProtocol_add("file", NO, HTLoadFile, NULL);
    HTProtocol_add("ftp", NO, HTLoadFTP, NULL);
    HTProtocol_add("nntp", NO, HTLoadNews, NULL);
    HTProtocol_add("news",NO, HTLoadNews, NULL);
    HTProtocol_add("gopher", NO, HTLoadGopher, NULL);

    HTConversion_add(context->conversions, "text/x-http",	"*/*",		  HTTPStatus_new,	1.0, 0.0, 0.0);
    HTConversion_add(context->conversions, "www/mime",       	"*/*",            HTMIMEConvert, 1.0, 0.0, 0.0);
    HTConversion_add(context->conversions, TEXT_DOCUMENT,    	"www/present",    HTXParse,    0.4, 0.0, 0.0);
    HTConversion_add(context->conversions, GIF_DOCUMENT,     	"www/present",    HTXParse,    0.6, 0.0, 0.0);
#ifdef JPEG
    HTConversion_add(context->conversions, JPEG_DOCUMENT,     	"www/present",    HTXParse,    0.8, 0.0, 0.0);
#endif /* JPEG */

#ifdef PNG
    HTConversion_add(context->conversions, PNG_DOCUMENT,     	"www/present",    HTXParse,    0.8, 0.0, 0.0);
    HTConversion_add(context->conversions, PNG_EXP_DOCUMENT,     	"www/present",    HTXParse,    0.8, 0.0, 0.0);
#endif /* PNG */
    HTConversion_add(context->conversions, XPM_DOCUMENT,		"www/present",    HTXParse,    0.6, 0.0, 0.0);
    HTConversion_add(context->conversions, XBM_DOCUMENT, 	"www/present",    HTXParse,    0.6, 0.0, 0.0);
    HTConversion_add(context->conversions, HTML_DOCUMENT,      	"www/present",    HTXParse,    0.6, 0.0, 0.0);
    HTConversion_add(context->conversions, HTML_LEVEL3_DOCUMENT,	"www/present",    HTXParse,    0.9, 0.0, 0.0);
    HTConversion_add(context->conversions, HTML3_DOCUMENT,   	"www/present",    HTXParse,    0.8, 0.0, 0.0);
    HTConversion_add(context->conversions, "application/octet-stream","www/present", HTSaveLocally, 0.1, 0.0, 0.0);
#if 0
    HTConversion_add(context->conversions, "application/x-compressed","www/present", HTSaveLocally, 0.1, 0.0, 0.0);
#endif
    HTConversion_add(context->conversions, "*/*",      	"www/present",    HTXParse,    0.1, 0.0, 0.0);
    HTConversion_add(context->conversions,"message/rfc822","*/*",HTMIMEConvert, 1.0, 0.0, 0.0);
    HTConversion_add(context->conversions,"multipart/*", "*/*", HTBoundary, 1.0, 0.0, 0.0);
    HTConversion_add(context->conversions,"www/unknown", "*/*",	HTGuess_new, 1.0, 0.0, 0.0);
#if 0
    HTConversion_add(context->conversions,"*/*","www/debug",HTBlackHoleConverter,1.0, 0.0, 0.0);
#endif

    HTAlert_add(AHTProgress, HT_A_PROGRESS); 
/*
    HTAlert_add(HTError_print, HT_A_MESSAGE);
    */
    HTBind_caseSensitive(FALSE);
    HTBind_addType("html", "text/html", 0.9);
    HTBind_addType("htm", "text/html", 0.9);
    HTBind_addType("gif", "image/gif", 0.9);
    HTBind_addType("png", "image/png", 0.9);
    HTBind_addType("jpg", "image/jpeg", 0.9);
    HTBind_addType("txt", "text/plain", 0.9);
    HTFileInit();

    if (!NoMailcap)
	register_mailcaps();

  
    if (CacheRoot) {
	HTCache_enable(CacheRoot);
	HTCache_setExpiresMode(HT_EXPIRES_AUTO, NULL);
    }
    else
	HTCache_disable();

    /* get atom values, so that we don't have to do strcmp for content types later */

    text_atom = HTAtom_caseFor(TEXT_DOCUMENT);
    html_atom = HTAtom_caseFor(HTML_DOCUMENT);
    html3_atom = HTAtom_caseFor(HTML3_DOCUMENT);
    html_level3_atom = HTAtom_caseFor(HTML_LEVEL3_DOCUMENT);
    gif_atom = HTAtom_caseFor(GIF_DOCUMENT);
    jpeg_atom = HTAtom_caseFor(JPEG_DOCUMENT);
    png_atom = HTAtom_caseFor(PNG_DOCUMENT);
    xpm_atom = HTAtom_caseFor(XPM_DOCUMENT);
    xbm_atom = HTAtom_caseFor(XBM_DOCUMENT);

}


void HTCallClient(HTXParseStruct * eps)
{
    Doc *doc = NULL;
    HTAnchor *anchor = NULL;
    HTRequest *req = eps->request;

    if (req->childAnchor)
	anchor = (HTAnchor *) req->childAnchor;
    else if (req->anchor)
	anchor = (HTAnchor *) req->anchor;

    if (anchor == NULL){
	if(BRIDGE_TRACE)
	    fprintf(stderr,"HTCallClient: NO ANCHOR FOUND, returning\n");
	return;
    }

    if (anchor->parent == NULL){ /* Brian Campbell <brianc@qnx.com> 16-Mar-96 */
	if(BRIDGE_TRACE)
	    fprintf(stderr,"HTCallClient: NO ANCHOR parent FOUND, returning\n");
	return;
    }
    doc = HTRequest_context(req);

#if 0
    /* if there has been a redirection, the anchor will have changed
       and we need to go throught the history list */

    if (req->redirectionAnchor) {
        History *h;
	HTList *l = context->history;

	while ((h = (History *)HTList_nextObject(l))) {
	    if (h->anchor->parent == doc->anchor) {
	        h->anchor = (req->childAnchor ? (HTAnchor *)req->childAnchor : (HTAnchor *)req->anchor);
		doc->anchor = req->anchor->parent;
	    }
	}
    }
#endif
    if (doc == NULL){
	if (BRIDGE_TRACE)
	    fprintf(stderr,"HTCallClient: NO DOC FOUND IN CONTEXT ???\n"); 
	return;
    }
    doc->url = anchor->parent->address;

    if (eps->finished) {

#if 0 /* null-termination now done by library */
/*	doc->request = eps->request; */
	/* NULL-terminating this seems necessary.. do we cross boundaries?? */
	eps->buffer[eps->used] = 0;   

	if (BRIDGE_TRACE)
	    fprintf(stderr,"HTCallClient finishing %s, got %d bytes\n",doc->url,eps->used);
#endif
	doc->content_buffer = eps->buffer;


    } else {
	if (BRIDGE_TRACE && VERBOSE_TRACE) {
	    fprintf(stderr,"HTCallClient %s, read %d bytes %lx\n",
		    doc->url, eps->used, doc->anchor);
	}
	doc->state = DOC_PENDING;
	doc->loaded_length = eps->used;

	if (initialised){
#ifdef PROG_PNG
	    if((HTAnchor_format(anchor) == png_atom) || (HTAnchor_format(anchor) == png_exp_atom))
	    {
		doc->content_buffer = eps->buffer;
		if(doc->loaded_length < HTAnchor_length(doc->anchor->parent))
		    doc->image = ProcessProgressiveImage(doc);
		else
		    doc->image = ProcessLoadedImage(doc);
		Announce("PNG processed\n");
		if(doc->main_doc)
		{
		    CurrentDoc = doc;
		    CurrentDoc->show_raw = FALSE;
		    NewBuffer(CurrentDoc);
		    HistoryVerify((HTAnchor *)doc->anchor);
		    DeltaHTMLPosition(context->current_history->y);
		    DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
		    SetScrollBarHPosition(PixelIndent, buf_width);
		    SetScrollBarVPosition(context->current_history->y, buf_height);
		    DisplayScrollBar();
		    DisplayUrl();
		} 
		else
		{
		    if (doc->host_anchors) {
			if (BRIDGE_TRACE) {
			    fprintf(stderr,"    HTEventRequestTerminate EVENT_OK inline\n");
			}
			
			NewBuffer(CurrentDoc);
			if (!CurrentDoc->tag)
			    DeltaHTMLPosition(context->current_history->y);
			DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
			if (!CurrentDoc->tag) {
			    SetScrollBarHPosition(PixelIndent, buf_width);
			    SetScrollBarVPosition(context->current_history->y, buf_height);
			}
			DisplayScrollBar();
			DisplayUrl();
		    }
		}
	
	    }
#endif
	    if (doc->loaded_length > 1024) {
		long int len = HTAnchor_length(doc->anchor->parent);
		int a, b;

		/* first, compute the k lengths */

		a = (doc->loaded_length + 512) / 1024;
		b = (len + 512) / 1024;

		if (len > 0) {     /* if content-length was given */
		    int left = len - doc->loaded_length;
		    if (a == b && left > 0) {
			Announce("Read %dk (%d bytes remain) from %s", a, left, doc->url);	
		    } else {
			Announce("Read %dk/%dk from %s", a, b, doc->url);
		    }
		 }
		 else
		    Announce("Read %dk of %s", (int)((doc->loaded_length + 512) / 1024), doc->url);

	    }
	}
    }
    return;
}



int timeout_handler (HTRequest * request)
{
/*     fprintf(stderr, "timeout_handler: do something cool!!\n");*/
/*    Announce("Connection timeout!"); */
    HideBusy();
    return 0;
}




int terminate_handler (HTRequest * request, int status) 
{
    Doc *doc;
    HTAnchor *anchor = NULL;

/*     fprintf(stderr, "terminate_handler, status = %d\n", status);*/

    switch (status) {
    case HT_ERROR:
	doc = (Doc *) HTRequest_context(request);
#if 0
	if (doc->main_doc) {
	    CurrentDoc = doc;
	    CurrentDoc->show_raw = FALSE;
	    NewBuffer(CurrentDoc);
	    DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
	    SetScrollBarHPosition(PixelIndent, buf_width);
	    SetScrollBarVPosition(context->current_history->y, buf_height);
	    DisplayScrollBar();
	    DisplayUrl();
	}
#endif
	if(doc->request->redirectionAnchor)
	{
	    redirection_handler(doc->request, status);
	    return HT_OK;
	}
	Announce("Error while fetching %s", doc->url);
	HideBusy();
	HistoryDelete((HTAnchor *)doc->anchor);
	return HT_OK;
	break;

      case HT_LOADED:
	if (BRIDGE_TRACE && VERBOSE_TRACE)
	    fprintf(stderr,"terminate_handler HT_LOADED\n");

	doc = (Doc *) HTRequest_context(request);
	if (doc == NULL) {
	    if(BRIDGE_TRACE)
		fprintf(stderr,"terminate_handler called before Doc set\n");
	    return EXIT_SUCCESS;
	} /* 12-Mar-96 Herman ten Brugge <herman@htbrug.hobby.nl> */
	if (doc->state < DOC_LOADED)
	    doc->state = DOC_LOADED;

	if (doc->main_doc) {

	    /* we will get to this point even when the document was
               handled by an external application registered through
               mailcap. The only (?) way to detect this is if
               content_buffer is NULL */

	    if (doc->content_buffer == NULL) {
		HideBusy();
		HistoryDelete((HTAnchor *)doc->anchor);
		DocsFree(doc);
		return EXIT_SUCCESS;
	    }

	    if (BRIDGE_TRACE) {
#if defined (__alpha) && defined (__osf__)
		fprintf(stderr,"    HTEventRequestTerminate register: %s %p\n", doc->url, doc->anchor);
#else
		fprintf(stderr,"    HTEventRequestTerminate register: %s %lx\n", doc->url, doc->anchor);
#endif /* (__alpha) && (__osf__) */
	    }


	    if (doc != CurrentDoc) {
		if (doc->anchor == CurrentDoc->anchor) {
		    doc->pending_reload = CurrentDoc->pending_reload;
		    doc->nofree = CurrentDoc->nofree;
		    doc->user_style = CurrentDoc->user_style;
		}

		DocsFree(CurrentDoc);
		CurrentDoc = doc;

		if (CurrentDoc->anchor->parent == context->home_anchor->parent)
		    CurrentDoc->user_style = TRUE;

	    }
	    
/*	    CurrentDoc->tag = tag;*/

	    /* at this point, we should verify the history
               registeration done in libGetDocument */

	    HistoryVerify(doc->anchor);


	    /* if an external view shows an image, the content_buffer will be NULL */

	    if (!CurrentDoc->content_buffer)
		CurrentDoc->loaded_length = 0;

	    NewBuffer(doc);

	    /* if the document hasn't been processed by now, it never
	       will be. Since we've already changed CurrentDoc, we need to change it
	       back. This is easiest with BackDoc() */

	    if (doc->state != DOC_PROCESSED) {
		BackDoc();
	        HistoryDelete(anchor);
		Announce("Unable to display %s",doc->url);
		HideBusy();
		return HT_OK;
	    }      

	    /* if the document has been reloaded, turn off flag to
	       avoid looping with inlines */

	    CurrentDoc->pending_reload = FALSE;

	    if (!CurrentDoc->tag && context->current_history->y)
		DeltaHTMLPosition(context->current_history->y);
	    DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
	    if (!CurrentDoc->tag) {
		SetScrollBarHPosition(PixelIndent, buf_width);
		SetScrollBarVPosition(context->current_history->y, buf_height);
	    }
	    DisplayScrollBar();
	    DisplayUrl();
	    HideBusy();

	} else { /* i.e. !main_doc */

	    /* this is an inline finishing -- first check to see that
               the host_anchors exists, in it the hostimg document
               should be found */

	    if (doc->host_anchors) {
		if (BRIDGE_TRACE) {
		    fprintf(stderr,"    HTEventRequestTerminate EVENT_OK inline\n");
		}

		NewBuffer(CurrentDoc);
		if (!CurrentDoc->tag)
		    DeltaHTMLPosition(context->current_history->y);
		DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
		if (!CurrentDoc->tag) {
		    SetScrollBarHPosition(PixelIndent, buf_width);
		    SetScrollBarVPosition(context->current_history->y, buf_height);
		}
		DisplayScrollBar();
		DisplayUrl();

	    } else
		if (BRIDGE_TRACE)
		    fprintf(stderr,"    HTEventRequestTerminate: WHOOPS no host_doc specified\n");

	    if (CurrentDoc)
		DisplayUrl();
	}
	break;
    default:
        break;
    }
/*    ReqDel(request); */ /* howcome 19/2/96: we can't delete the request here, but where?? */
    return HT_OK;
	
}





/* libEventLoop is the final call from the application. It should only
   exit when the application is being exited */


void libEventLoop(int c, char *href)
{
    char * ref = NULL;
    Doc *doc;
    HTAnchor *anchor;
    Doc *init_doc; /* must be visiable to style.c to determine if inital doc is being loaded */
    HTRequest *request = HTRequest_new(); 

    /* register socket for X events */
    /*
    ecb.sockfd = c;
    ecb.callback = libCallback;
    HTEventRegister(&ecb);
    */

    {
        struct timeval tv;
        tv.tv_sec = SEC_TIMEOUT;	      /* Default timeout for sockets */
	tv.tv_usec = USEC_TIMEOUT;

	/* Set timeout on sockets */
	HTEvent_registerTimeout(&tv, request, timeout_handler, NO);
    }



    /* having to submit a request structure here seems stupid */

    HTEvent_Register(c, request, (SockOps)FD_READ, EventHandler, 0) ;

    /* display initial page */

    init_doc = DocNew(); /* free this at some point */
    init_doc->main_doc = TRUE;
    init_doc->url = NULL;
    init_doc->nofree = TRUE;
    init_doc->content_buffer = INITIAL_HTML;
    init_doc->tag = NULL;
    init_doc->already_displayed = FALSE;
    init_doc->source_editor = NULL;
    init_doc->field_editor = NULL;
    init_doc->edited_field = NULL;

    /* get atom values, so that we don't have to do strcmp for content types later */

    text_atom = HTAtom_caseFor(TEXT_DOCUMENT);
    html_atom = HTAtom_caseFor(HTML_DOCUMENT);
    html3_atom = HTAtom_caseFor(HTML3_DOCUMENT);
    html_level3_atom = HTAtom_caseFor(HTML_LEVEL3_DOCUMENT);
    gif_atom = HTAtom_caseFor(GIF_DOCUMENT);
    jpeg_atom = HTAtom_caseFor(JPEG_DOCUMENT);
    png_atom = HTAtom_caseFor(PNG_DOCUMENT);
    png_exp_atom = HTAtom_caseFor(PNG_EXP_DOCUMENT);
    xpm_atom = HTAtom_caseFor(XPM_DOCUMENT);
    xbm_atom = HTAtom_caseFor(XBM_DOCUMENT);

    CurrentDoc = init_doc;
    NewBuffer(init_doc);
	
    DisplayAll();
    

    GuiEvents(0, NULL, 0);
    GuiEvents(0, NULL, 0);
    GuiEvents(0, NULL, 0);

    HTNetCall_addBefore(HTLoadStart, 0);
    HTNetCall_addAfter(redirection_handler, HT_PERM_REDIRECT);
    HTNetCall_addAfter(redirection_handler, HT_TEMP_REDIRECT);
    HTNetCall_addAfter(terminate_handler, HT_ALL);

    HTDNS_setTimeout(300);

    /* now do home page */


    anchor = libExpandHref(href, strlen(href));
    anchor->parent->document = init_doc;

    context->home_anchor = anchor->parent;
    doc = libLoadAnchor(anchor, NULL, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE);


    /* enter infinite loop */

    HTEvent_Loop(doc->request);
    fprintf(stderr,"libeventloop exiting, why??\n");
    Free(ref);
}



void libExit(int pCache)
{
    if (BRIDGE_TRACE)
        fprintf(stderr,"libExit cleaning up\n");

    ReqDelAll();

#if 0
    if (!pCache)
        HTCache_deleteAll();

    HTList_free (reqlist);

    if (context) {
	l = context->registered_anchors;

#if 0
#ifndef DOCFREE_BUG
	/* for some reason, the fails on sgi's */

	while (a = (HTParentAnchor *)HTList_nextObject(l)) {
	    if (a->document)
		DocFree((Doc *)a->document, TRUE);
	}
#endif
#endif
	HTList_free (context->registered_anchors);
	HTList_free (context->conversions);
    }

#endif

    HTLibTerminate();
}




