#include <stdio.h>
#include <stdarg.h>

#include "www.h"

extern Context *context;
extern int debug;
extern Doc *CurrentDoc;

extern int sbar_width;
extern unsigned int win_width, win_height;
extern int statusHeight;
extern int ToolBarHeight;

extern long buf_height;
extern int buf_width;
extern int PixelIndent;

/* The multithreaded library complicates the history
   mechannism. Several documents may be requested simultaneously, and
   they may arrive in a different order than they were clicked
   in. Therefore, putting a document into the history is a two-stem
   porcess: when the user clicks, HistoryRecord put it into the
   history, but does not enable it -- then, when a document arrives,
   HistoryVerify enables it. */

History *NewHistory()
{
    History *h;

    h = ((History *) calloc (sizeof(History), 1));
    if (!h)
	return NULL;
    h->state = HISTORY_NOTREGISTERED;
    h->tag = NULL;
    h->y = 0;
    return h;
}

/* debugging routine which prints out the current history stack */

void HistoryList()
{
    HTList *ll, *l = context->history;
    History *h;
    HTAnchor *a;

    ll = l;
    fprintf(stderr,"\n+++++++ list history, current position %d n_ele %d\n", context->history_pos, HTList_count(context->history));

    while ( (h = (History *)HTList_nextObject(l)) ) {
	a = h->anchor;
#if defined PRINTF_HAS_PFORMAT
	fprintf(stderr," h=%p anchor=%p state %d ",h,a,h->state);
#else
	fprintf(stderr," h=%lx anchor=%lx state %d ",h,a,h->state);
#endif /* PRINTF_HAS_PFORMAT */
	if (h->title)
	    fprintf(stderr,"h->title=\"%s\"",h->title);
	if (a->parent->physical)
	    fprintf(stderr,"%s",a->parent->physical);
	else if (a->parent->address)
	    fprintf(stderr,"%s",a->parent->address);
	else if (a->parent->document) {
	    Doc *d = (Doc *)a->parent->document;
	    fprintf(stderr,"%s",d->url);
	}

	if ((HTAnchor *)a->parent == a)
	    fprintf(stderr," PARENT ");
	else
	    fprintf(stderr," CHILD: %s ",h->tag);

	fprintf(stderr," list position %d\n",HTList_indexOf(ll,h));
    }
    fprintf(stderr,"+++++++ end list history\n\n");
}

/* go to home document */

void HomeDoc()
{
    if (context->home_anchor->parent->document) {
	if (HISTORY_TRACE)
	    fprintf(stderr,"HomeDoc\n");

	context->history_pos = HTList_count (context->history) - 1;
	context->current_history = (History *)HTList_objectAt(context->history, context->history_pos);

	CurrentDoc = (Doc *) context->home_anchor->parent->document;
	CurrentDoc->show_raw = FALSE;
	NewBuffer(CurrentDoc);
	DeltaHTMLPosition(context->current_history->y);
	DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
	SetScrollBarHPosition(PixelIndent, buf_width);
	SetScrollBarVPosition(context->current_history->y, buf_height);
	DisplayScrollBar();
	Announce(CurrentDoc->url);
    }
    else {
	if (HISTORY_TRACE)
	    fprintf(stderr,"HomeDoc: home_doc freed??????\n");
	libLoadAnchor((HTAnchor *)context->home_anchor, NULL, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE);
    }

    if (HISTORY_TRACE)
	HistoryList();
}


void ForwardDoc()
{
    History *h = NULL;
    HTAnchor *a;
    Doc *d;
    int pos; 

    if (HISTORY_TRACE)
	fprintf(stderr,"->ForwardDoc\n");

    pos = context->history_pos;

    while (pos > 0) {
	pos--;
	h = (History *)HTList_objectAt(context->history, pos);
	
	if (!h) {
	    if (HISTORY_TRACE)
		fprintf(stderr,"ForwardDoc: no history to be found \n");
	    continue;
	}

	if (h->state == HISTORY_VERIFIED) {
	    break;
	}

	if (HISTORY_TRACE)
	    fprintf(stderr,"ForwardDoc: selected item either not loaded or deleted %d\n",h->state);	

	h = NULL;

    }

    if (!h) {
	if (HISTORY_TRACE)
	    fprintf(stderr,"ForwardDoc: no history to be found \n");
	return;
    }

    /* from here we assume success */

    context->history_pos = pos;
    context->current_history = (History *)HTList_objectAt(context->history, context->history_pos);
    a = h->anchor;
	
    if (HISTORY_TRACE)
	fprintf(stderr,"ForwardDoc, pos = %d, a = %s\n", context->history_pos, a->parent->physical);
    
    if ((d = (Doc *) a->parent->document)) {
	if (d->state >= DOC_LOADED) {
	    CurrentDoc = (Doc *) d;
	    CurrentDoc->tag = h->tag;
	    CurrentDoc->show_raw = FALSE;
	    NewBuffer(CurrentDoc);

	    /* if there is a tag, the positioning is being taken care of */

	    if (!h->tag)
		DeltaHTMLPosition(context->current_history->y);

	    DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);

	    if (!h->tag) {
		SetScrollBarHPosition(PixelIndent, buf_width);
		SetScrollBarVPosition(context->current_history->y, buf_height);
	    }
	    DisplayScrollBar();
	    DisplayUrl();
	}
	else {
	    fprintf(stderr,"ForwardDoc: WEIRD, doc is reg'd but state is low\n");
	}
    } else {
	libLoadAnchor(a, NULL, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE);
    }

    if (HISTORY_TRACE)
	HistoryList();
}

void BackDoc()
{
    History *h = NULL;
    HTAnchor *a;
    Doc *d;
    int pos; 

    if (HISTORY_TRACE)
	fprintf(stderr,"->BackDoc\n");

    pos = context->history_pos;

    while (pos < (HTList_count(context->history) - 1)) {
	pos++;
	h = (History *)HTList_objectAt(context->history, pos);
	
	if (!h) {
	    if (HISTORY_TRACE)
		fprintf(stderr,"BackDoc: no history to be found \n");
	    continue;
	}

	if (h->state == HISTORY_VERIFIED) {
	    break;
	}

	if (HISTORY_TRACE)
	    fprintf(stderr,"BackDoc: selected item not yet confirmed loaded %d\n",h->state);	

	h = NULL;
    }

    if (!h) {
	if (HISTORY_TRACE)
	    fprintf(stderr,"BackDoc: no history to be found \n");
	return;
    }

    /* from here we assume success */

    context->history_pos = pos;
    context->current_history = (History *)HTList_objectAt(context->history, context->history_pos);
    a = h->anchor;

    if (HISTORY_TRACE)
	fprintf(stderr,"BackDoc, pos = %d, a = %s\n", context->history_pos, a->parent->physical);
    
    if ((d = (Doc *) a->parent->document)) {
	if (d->state >= DOC_LOADED) {
	    CurrentDoc = (Doc *) d;
	    CurrentDoc->tag = h->tag;
	    CurrentDoc->show_raw = FALSE;
	    NewBuffer(CurrentDoc);
	    DeltaHTMLPosition(context->current_history->y);
	    DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
	    SetScrollBarHPosition(PixelIndent, buf_width);
	    SetScrollBarVPosition(context->current_history->y, buf_height);
	    DisplayScrollBar();
	    DisplayUrl();
	}
	else {
	    fprintf(stderr,"BackDoc: WEIRD, doc is reg'd but state is low\n");
	}
    } else {
	libLoadAnchor(a, NULL, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE);
    }

    if (HISTORY_TRACE)
	HistoryList();
}



void GotoDoc(int pos)
{
    History *h = NULL;
    HTAnchor *a;
    Doc *d;

    if (HISTORY_TRACE)
	fprintf(stderr,"->ForwardDoc\n");

    h = (History *)HTList_objectAt(context->history, pos);
	
    if (!h) {
	if (HISTORY_TRACE)
	    fprintf(stderr,"BackDoc: no history to be found \n");
	return;
    }

    if (h->state != HISTORY_VERIFIED) {
	if (HISTORY_TRACE)
	    fprintf(stderr,"BackDoc: selected item not yet confirmed loaded %d\n",h->state);	
	return;
    }

    /* from here we assume success */

    context->history_pos = pos;
    context->current_history = (History *)HTList_objectAt(context->history, context->history_pos);
    a = h->anchor;

    if (HISTORY_TRACE)
	fprintf(stderr,"BackDoc, pos = %d, a = %s\n", context->history_pos, a->parent->physical);
    
    if ((d = (Doc *) a->parent->document)) {
	if (d->state >= DOC_LOADED) {
	    CurrentDoc = (Doc *) d;
	    CurrentDoc->tag = h->tag;
	    CurrentDoc->show_raw = FALSE;
	    NewBuffer(CurrentDoc);
	    DeltaHTMLPosition(context->current_history->y);
	    DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
	    SetScrollBarHPosition(PixelIndent, buf_width);
	    SetScrollBarVPosition(context->current_history->y, buf_height);
	    DisplayScrollBar();
	    DisplayUrl();
	}
	else {
	    fprintf(stderr,"BackDoc: WEIRD, doc is reg'd but state is low\n");
	}
    } else {
	libLoadAnchor(a, NULL, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE /*TRUE*/);
    }

    if (HISTORY_TRACE)
	HistoryList();
}





void HistoryRecord(HTAnchor *a)
{
    History *h;
    int i;

    if (HISTORY_TRACE)
#if defined PRINTF_HAS_PFORMAT
	fprintf(stderr, "->HistoryRecord %p\n",a);
#else
	fprintf(stderr, "->HistoryRecord %lx\n",a);
#endif /* PRINTF_HAS_PFORMAT */

    /* we're putting a new doc into the history list. First mark the
       rest of the history for death since this is a deleting history
       mechanism */

    i = context->history_pos - 1;
    while(i >= 0) {
	if (HISTORY_TRACE)
	    fprintf(stderr, "HistoryRecord: in loop %d\n",i);	    
	h = (History *)HTList_objectAt(context->history, i);
	if (h && h->state == HISTORY_VERIFIED) {
	    h->state = HISTORY_DELETED;
	    if (HISTORY_TRACE)
		fprintf(stderr, "HistoryRecord: deleting %d\n",i);	    
	}
	i--;
    }

    /* then, create a new history record to be put onto the history */

    h = NewHistory();
    h->state = HISTORY_REGISTERED;
    h->anchor = a;

    if ((HTAnchor *)a->parent != a) {
	HTChildAnchor *c = (HTChildAnchor *) a;

	h->tag = c->tag;
	if (HISTORY_TRACE)
	    fprintf(stderr,"HistoryRecord: recording CHILS tag %s\n", c->tag);
    }    

    if (!context->history)
	context->history = HTList_new();
    HTList_addObject(context->history, h);

    context->history_pos++;
    context->current_history = (History *)HTList_objectAt(context->history, context->history_pos);
}

void HistoryDelete(HTAnchor *a)
{
    /* janet 21/07/95: not used:   Doc *d; */
    History *h;
    HTList *l = context->history;

    if (HISTORY_TRACE)
#if defined PRINTF_HAS_PFORMAT
	fprintf(stderr,"->HistoryDelete %p\n",a);
#else
	fprintf(stderr,"->HistoryDelete %lx\n",a);
#endif /* PRINTF_HAS_PFORMAT */

    while ( (h = (History *)HTList_nextObject(l)) ) {
	if (a == h->anchor) {
	    h->state = HISTORY_DELETED;
	    if (HISTORY_TRACE)
		fprintf(stderr,"HistoryDelete %s\n",a->parent->address);
	}
    }

    if (HISTORY_TRACE)
	HistoryList();
}


void HistoryVerify(HTAnchor *a)
{
    /* janet 21/07/95: not used:    Doc *d; */
    History *h;
    HTList *l;
    int count = 0;
    int i;

    if (HISTORY_TRACE)

#if defined PRINTF_HAS_PFORMAT
	fprintf(stderr, "->HistoryVerify %p\n",a);
#else
	fprintf(stderr, "->HistoryVerify %lx\n",a);
#endif /* PRINTF_HAS_PFORMAT */

    /* the incoming anchor has been loaded and should be verified on
       the history list. As it is verified, we should delete the rest
       of the branch. The mark-for-death had to be on at a stage not
       called if we were just moving around in history,
       i.e. HistoryRecord */

    i = 0;
    l = context->history;
    while ( (h = (History *)HTList_nextObject(l)) ) {
	if (h->state == HISTORY_DELETED) {
	    if (HISTORY_TRACE) {
		HTAnchor *a = h->anchor;
		fprintf(stderr, "*****libHistoryVerify: deleting %d %s\n",i,a->parent->physical);
	    }
	    l = l->next; /* since we'll remove the object l is pointing to */
	    HTList_removeObjectAt(context->history, i);  /* dumps here if error on start_up */
	} else /* if we just deleted element, it moved ahead automatically */
	    i++;
    }	

    l = context->history;
    while ( (h = (History *)HTList_nextObject(l)) ) {
	if (h->anchor == a) {
	    context->history_pos = count;
	    context->current_history = (History *)HTList_objectAt(context->history, context->history_pos);
	    if (h->state == HISTORY_REGISTERED) {
		h->state = HISTORY_VERIFIED;
		if (CurrentDoc->title)
		    h->title = strdup(CurrentDoc->title);
		if (HISTORY_TRACE)
		    fprintf(stderr, "-->HistoryVerify: context->history_pos = %d\n", context->history_pos);
	    }
	    else
		if (HISTORY_TRACE)
		    fprintf(stderr, "libHistoryVerify: check STATE \n");
	}
	count++;
    }	
    if (HISTORY_TRACE)
	HistoryList();
}

