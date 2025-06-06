/*


   Almost everything about editor operations
   (c) Nov 1995 By Yves Lafon for INRIA / W3C


*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "types.h"

EditorBuffer *CreateBuffer()
{
    EditorBuffer *new_buffer;

    new_buffer = (EditorBuffer *) malloc(sizeof(EditorBuffer));
    if(new_buffer)
    {
	new_buffer->cell = NULL;
	new_buffer->pos = 0;
	new_buffer->size = 0;
	return(new_buffer);
    };
    printf("Ran out of memory in CreateBuffer\n");
    exit(0);
}

cell *CreateCell()
{
    cell *new_cell;
    
    new_cell = (cell *) malloc(sizeof(cell));
    if(new_cell) /* did this to avoid gcc warning */
    {
	new_cell->bank = (char *) calloc(BANK_SIZE+1,sizeof(char));
	if(new_cell->bank)
	{
	    new_cell->next = NULL;
	    new_cell->prev = NULL;
	    new_cell->size = 0;
	    return(new_cell);
	};
    };
    printf("Ran out of memory in CreateCell\n");
    exit(0);    
}

void FreeCell(cell *TheCell)
{
    if(TheCell)
    {
	if(TheCell->bank)
	    free(TheCell->bank);
	free(TheCell);
    };
}

void FreeBuffer(EditorBuffer *buffer)
{
    cell *wiped_cell;
    
    if(buffer)
    {
	wiped_cell = buffer->cell;
	if(wiped_cell)
	{
	    if(wiped_cell->next)
	    {
		wiped_cell=wiped_cell->next;
		while(wiped_cell->next)
		{
		    FreeCell(wiped_cell->prev);
		    wiped_cell = wiped_cell->next;
		};
		FreeCell(wiped_cell->prev);
	    };
	    FreeCell(wiped_cell);
	};
	free(buffer);
    };
}

void SplitCell(cell *buffer_cell)
{
    cell *new_cell;
    int i;
    char *oldbank,*newbank;
    
    new_cell = CreateCell();
    new_cell->prev = buffer_cell;
    new_cell->next = buffer_cell->next;
    buffer_cell->next = new_cell;
    if(new_cell->next)
	new_cell->next->prev = new_cell;
    new_cell->size = BANK_SIZE - SPLIT_SIZE ;
    buffer_cell->size = SPLIT_SIZE;
    oldbank = buffer_cell->bank + SPLIT_SIZE;
    newbank  = new_cell->bank;
    for(i=0;i< BANK_SIZE - SPLIT_SIZE;i++)
	*newbank++ = *oldbank++;
}

void MergeCell(cell *origin)
{
    int i,size;
    char *o_bank,*w_bank;
    cell *wiped;

    if(origin)
	if(origin->next)
	    if(origin->size + origin->next->size < BANK_SIZE)
	    {
		wiped = origin->next;
		size = wiped->size;
		o_bank = origin->bank + origin->size;
		w_bank = wiped->bank;
		for(i=0;i<size;i++)
		    *o_bank++=*w_bank++;
		origin->size+=wiped->size;
		origin->next = wiped->next;
		if(origin->next)
		    origin->next->prev = origin;
		FreeCell(wiped);
	    };
}

int DeleteChar(EditorBuffer *buffer, int pos)
{
    cell *buffer_cell;
    int i,j;
    char *position;
    
    buffer_cell=buffer->cell;

    if(buffer_cell)
    {
	for(i=pos;(i>=buffer_cell->size)&&buffer_cell;)
	{
	    i-=buffer_cell->size;
	    if(i)
		buffer_cell=buffer_cell->next;
	    else
	    {
		if(buffer->size)
		    --buffer->size;
		if(buffer->pos>buffer->size)
		    buffer->pos=buffer->size;
		return(1);
	    };
	};
	if(!buffer_cell)
	    return(0);  /* error, ran out of editor */
	
	if(buffer_cell->size>1)
	{
	    if(i<BANK_SIZE-1) /* if it is not at the end, we update */
	    {
		j = buffer_cell->size;
		for(position=buffer_cell->bank+i+1;i<=j;i++,position++)
		    *(position-1)=*position;
	    }
	    else
	    {
		if(buffer->pos)
		    --buffer->pos;	
	    };
	    --buffer_cell->size;  /* update bank size */
	    MergeCell(buffer_cell);  /* optimize bank filling if needed */
	    if(buffer_cell->prev)
		MergeCell(buffer_cell->prev);
	}
	else /* size 1 -> we remove the bank */
	{
#if 1
	    if(buffer_cell->prev)
		buffer_cell->prev->next = buffer_cell->next;
	    else
		buffer->cell = buffer_cell->next;
#else
	    ((buffer_cell->prev) ? buffer_cell->prev->next : buffer->cell) = buffer_cell->next;
#endif /* __STRICT_ANSI__ */
	    if(buffer_cell->next)
		buffer_cell->next->prev = buffer_cell->prev;
	    FreeCell(buffer_cell); 
	};
	if(buffer->size)
	    --buffer->size; /* update buffer size */
	if(buffer->pos>buffer->size)
	    buffer->pos=buffer->size;
	return(1);
    }
    return(0);
}

int InsertChar(EditorBuffer *buffer, int pos, char c)
{
    cell *buffer_cell;
    int i,j,b_size;
    char *position;

    if(!buffer)
	buffer = CreateBuffer();

    buffer_cell = buffer->cell;

    if(!buffer_cell)
    {
	buffer->cell = CreateCell();
	buffer_cell  = buffer->cell;
    };
    
    for(i=pos;(i>buffer_cell->size)&&buffer_cell;buffer_cell=buffer_cell->next)
	i-=(buffer_cell->size+1);
    
    if(!buffer_cell)
	return(0);  /* error, ran out of editor */
    
    b_size = buffer_cell->size;

    if(b_size == BANK_SIZE)  /* if max size -> split */
	SplitCell(buffer_cell);
    
    position = buffer_cell->bank + buffer_cell->size;
    for(j=buffer_cell->size;j>=i;j--,position--)
	*(position+1)=*position;
    *(buffer_cell->bank+i) = c;
    ++buffer_cell->size; /* update bank size */
    ++buffer->size;     /* update buffer size */
    
    return(1);
}

int InsertnChar(EditorBuffer *buffer, int pos, char *s, int size)
{
    int ok,i;
    char *s_pos;
    
    s_pos = s;
    
    for(ok=1,i=0;ok&&(i<size);i++)
	ok = InsertChar(buffer,pos+i,*s_pos++);
    
    return(ok);
}

int InsertString(EditorBuffer *buffer, int pos, char *s)
{
    int ok,size,i;
    char *s_pos;
    
    size = strlen(s);
    s_pos = s;

    for(ok=1,i=0;ok&&(i<size);i++)
	ok = InsertChar(buffer,pos+i,*s_pos++);

    return(ok);
}

int AppendChar(EditorBuffer *buffer, char c)
{
    return(InsertChar(buffer,buffer->size,c));
}

int AppendnChar(EditorBuffer *buffer, char *c, int size)
{
    return(InsertnChar(buffer,buffer->size,c,size));
}

int AppendString(EditorBuffer *buffer, char *s)
{
    return(InsertString(buffer,buffer->size,s));
}
    
char *Buffer2Str(EditorBuffer *buffer)
{
    char *string,*pos;
    cell *cell_link;

    if(buffer)
    {
	string = (char *)calloc(buffer->size+2,sizeof(char));
	
	if(!string)
	{
	    printf("Ran out of memory in Buffer2Str\n");
	    exit(0);
	};
	pos = string;
	for(cell_link=buffer->cell;cell_link;cell_link=cell_link->next)
	{
	    strncpy(pos,cell_link->bank,cell_link->size);
	    pos+=cell_link->size;
	};
	*pos=0; /* end of string */
	return(string);
    }
    return(NULL);
}

EditorBuffer *Str2Buffer(char *string)
{
    cell *cell_link;
    int size;
    char *pos;
    EditorBuffer *buffer;

    buffer = CreateBuffer();
    size = strlen(string);
    buffer->size = size;

    if(size)
    {
	cell_link = CreateCell();
	buffer->cell = cell_link;

	for(pos = string; size > BANK_SIZE ; size-=BANK_SIZE)
	{
	    strncpy(cell_link->bank,pos,BANK_SIZE);
	    pos+=BANK_SIZE;
	    cell_link->size = BANK_SIZE;
	    cell_link->next = CreateCell();
	    cell_link->next->prev = cell_link;
	    cell_link = cell_link->next;
	};
	strncpy(cell_link->bank,pos,size);
	cell_link->size = size;
    };
    return(buffer);
}

int LineNumber(EditorBuffer *buffer)
{
    int numline,i,j;
    cell *buffer_cell;

    numline=0;
    buffer_cell = (buffer) ? buffer->cell : NULL;
    for(i=0,j=0;(i<buffer->pos) && buffer_cell;i++,j++)
    {
	if(j>=buffer_cell->size)
	{
	    buffer_cell = buffer_cell->next;
	    j=0;
	}
	if(buffer_cell)
	    numline+=(*(buffer_cell->bank + j) == '\n');
    };
    return(numline);
}

int ColNumber(EditorBuffer *buffer)
{
    cell *buffer_cell;
    int i,j;
    int numcol;
    
    buffer_cell=buffer->cell;

    numcol=0;
    buffer_cell = (buffer) ? buffer->cell : NULL;
    for(i=0,j=0;(i<buffer->pos) && buffer_cell;i++,j++)
    {
	if(j>=buffer_cell->size)
	{
	    buffer_cell = buffer_cell->next;
	    j=0;
	}
	if(buffer_cell)
	    if(*(buffer_cell->bank + j) == '\n')
		numcol=0;
	    else
		numcol++;
    };
    return(numcol);
}

void NextLine(EditorBuffer *buffer)
{
    cell *buffer_cell;
    int i,j,done;
    int numcol,k;
    
    buffer_cell=buffer->cell;
    numcol = ColNumber(buffer);
    for(i=0,j=0;(i<buffer->pos) && buffer_cell;i++,j++)
    {
	if(j>=buffer_cell->size)
	{
	    buffer_cell = buffer_cell->next;
	    j=0;
	}
    }
    for(done=0;(i<buffer->size) && buffer_cell && !done; i++,j++)
    {
	if(j>=buffer_cell->size)
	{
	    buffer_cell = buffer_cell->next;
	    j=0;
	}
	if(buffer_cell)
	    if(*(buffer_cell->bank + j) == '\n')
		done = 1;
	    else
		++buffer->pos;
    };
    if(done)
    {
	for(done=0,k=0;(i<buffer->size) && buffer_cell && !done; i++,j++,k++)
	{
	    if(j>=buffer_cell->size)
	    {
		buffer_cell = buffer_cell->next;
		j=0;
	    }
	    if(buffer_cell)
	    {
		if((*(buffer_cell->bank + j) == '\n')||(k==numcol))
		    done = 1;
		
		++buffer->pos;
	    };
	};
    };
}

void PrevLine(EditorBuffer *buffer)
{
    cell *buffer_cell;
    int i,j,done;
    int numcol,numrow,k;
    
    numcol = ColNumber(buffer);
    numrow = LineNumber(buffer);
    buffer_cell=buffer->cell;
    if(numrow>1)
    {
	for(done=0,k=0,i=0,j=0;(i<buffer->size) && buffer_cell && !done; i++,j++)
	{
	    if(j>=buffer_cell->size)
	    {
		buffer_cell = buffer_cell->next;
		j=0;
	    }
	    if(buffer_cell)
		if(*(buffer_cell->bank + j) == '\n')
		{
		    k++;
		    done = (k == (numrow-1));
		};
	};
    }
    else
    {
	i=0;
	j=0;
	done=1;
    };
    buffer->pos = i;
    if(done)
    {
	for(done=0,k=0;(i<buffer->size) && buffer_cell && !done; i++,j++,k++)
	{
	    if(j>=buffer_cell->size)
	    {
		buffer_cell = buffer_cell->next;
		j=0;
	    };
	    if(buffer_cell)
	    {
		if((*(buffer_cell->bank + j) == '\n')||(k==numcol))
		    done = 1;
		else
		    ++buffer->pos;
	    };
	};
    };   
}
