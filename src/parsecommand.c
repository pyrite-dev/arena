#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>

#include "www.h"

char *ParseGoto(command_line, limit)
    char *command_line, *limit;
{
    char *curr;
    char *url = NULL;
    int  new;
    int  parse_ok;
    
    curr = command_line;
    parse_ok = FALSE;
    new = FALSE;
    while(curr <= limit && !parse_ok)
    {
	if(!strncmp(curr, "URL=", 4))
	{
	    url =  curr+4;
	}
	else if (!*curr)
	    parse_ok = TRUE;
 	else if (!strcmp(curr, "NEW"))
	    new = TRUE;
	curr += strlen(curr)+1;
    }
    if(parse_ok)
    {
	if(url)
	{
	    if(new)
	    {
		char *cmd;
		cmd = (char *)malloc(strlen(url)+9);
		sprintf(cmd, "arena %s &", url);
		system(cmd);
		free(cmd);
	    }
	    else
		OpenDoc(url);
	}
    }
    return limit;
}

void ParseCommandLine(command_line, command_size)
    char *command_line;
    int command_size;
{
    char *p, *end, *curr;

    end = command_line + command_size;
    curr = command_line;

    while(curr < end)
    {
	if(!strcmp(curr, "GOTO"))
	{
	    p = ParseGoto(curr+strlen(curr)+1, end);
	    curr = p;
	}
	else
	    curr += strlen(curr)+1;
    }
}
