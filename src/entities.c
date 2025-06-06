/* recognize HTML ISO entities */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define HASHSIZE 161

struct nlist
{
    struct nlist *next;
    char *name;
    unsigned code;
};

static struct nlist *hashtab[HASHSIZE];

struct entity
{
    char *name;
    unsigned code;
} entities[] = 
  {
    "nbsp",     32,      /* non breaking space */
    "quot",     34,
    "apos",     39,
    "iexcl",    161,
    "cent",     162,
    "pound",    163,
    "curren",   164,
    "yen",      165,
    "brvbar",   166,
    "sect",     167,
    "uml",      168,
    "copy",     169,
    "ordf",     170,
    "laquo",    171,
    "raquo",    187,
    "not",      172,
    "reg",      174,
    "macr",     175,
    "deg",      176,
    "plusmn",   177,
    "sup2",     178,
    "sup3",     179,
    "acute",    180,
    "micro",    181,
    "para",     182,
    "middot",   183,
    "cedil",    184,
    "sup1",     185,
    "ordm",     186,
    "frac14",   188,
    "frac12",   189,
    "iquest",   191,
    "frac34",   190,
    "AElig",    198,
    "Aacute",   193,
    "Acirc",    194,
    "Agrave",   192,
    "Aring",    197,
    "Atilde",   195,
    "Auml",     196,
    "Ccedil",   199,
    "ETH",      208,
    "Eacute",   201,
    "Ecirc",    202,
    "Egrave",   200,
    "Euml",     203,
    "Iacute",   205,
    "Icirc",    206,
    "Igrave",   204,
    "Iuml",     207,
    "Ntilde",   209,
    "Oacute",   211,
    "Ocirc",    212,
    "Ograve",   210,
    "Oslash",   216,
    "Otilde",   213,
    "Ouml",     214,
    "times",    215,
    "THORN",    222,
    "Uacute",   218,
    "Ucirc",    219,
    "Ugrave",   217,
    "Uuml",     220,
    "Yacute",   221,
    "aacute",   225,
    "acirc",    226,
    "aelig",    230,
    "agrave",   224,
    "amp",      38,
    "aring",    229,
    "atilde",   227,
    "auml",     228,
    "ccedil",   231,
    "eacute",   233,
    "ecirc",    234,
    "egrave",   232,
    "eth",      240,
    "euml",     235,
    "tagc",    	62,
    "gt",       62,
    "iacute",   237,
    "icirc",    238,
    "igrave",   236,
    "iuml",     239,
    "stago",    60,
    "lt",       60,
    "ntilde",   241,
    "oacute",   243,
    "ocirc",    244,
    "ograve",   242,
    "oslash",   248,
    "otilde",   245,
    "ouml",     246,
    "szlig",    223,
    "thorn",    254,
    "uacute",   250,
    "ucirc",    251,
    "ugrave",   249,
    "uuml",     252,
    "yacute",   253,
    "yuml",     255,

/* symbol characters used in maths start here */
/* Added by Janne Saarela (janne.saarela@cern.ch) */

    "divide",   247,
/*    <!ENTITY % ISOgrk3 PUBLIC
 *       "ISO 8879:1986//ENTITIES Greek Symbols//EN">
 *     %ISOgrk3;
 */
    
    "alpha",  97,
    "beta",   98,
    "gamma",  103,
    "Gamma",  71,
    "delta",  100,
    "Delta",  68,
    "epsi",   101,
    "zeta",   122,
    "eta",    104,
    "Theta",  81,
    "thetav", 74,
    "theta",  113,
    "thetas", 113,
    "iota",   105,
    "kappa",  107,
    "lambda", 108,
    "Lambda", 76,
    "mu",     109,
    "nu",     110,
    "xi",     120,
    "Xi",     88,
    "pi",     112,
    "piv",    118,
    "Pi",     80,
    "Psi",    121,
    "rho",    114,
    "sigma",  115,
    "Sigma",  83,
    "sigmav", 86,
    "tau",    116,
    "upsi",   117,
    "Upsi",   85,
    "phi",    102,
    "phis",   102,
    "Phi",    70,
    "phiv",   106,
    "chi",    99,
    "psi",    121,
    "omega",  119,
    "Omega",  87,

/*    <!ENTITY % ISOtech PUBLIC
 *      "ISO 8879-1986//ENTITIES General Technical//EN">
 *    %ISOtech;
 */

    "ap",     187,
    "and",    217,
    "cdot",   215, /* staalesc 28/07/95 */
    "cir",    176,
    "darr",   175,
    "dArr",   223,
    "empty",  198,
    "equiv",  186,
    "exist",  36,
    "forall", 34,
    "ge",     179,
    "harr",   171,
    "hArr",   219,
    "iff",    219,
    "inf",    165,
    "isin",   206,
    "lang",   225,
    "larr",   172,
    "lcub",   123,
    "lArr",   220,
    "le",     163,
    "ne",     185,
    "nabla",  209,
    "or",     218,
    "perp",   94,
    "prop",   181,
    "rang",   241,
    "rarr",   174,
    "rcub",   125,
    "rArr",   222,
    "sim",    126,
    "sub",    204,
    "sube",   205,
    "sup",    201,
    "supe",   202,
    "surd",   214, /* staalesc 13/12/95 */
    "shy",    173,
    "uarr",   173,
    "uArr",   221,
    "pd",     182
  };

static unsigned hash(char *s)
{
    unsigned hashval;

    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31*hashval;

    return hashval % HASHSIZE;
}

static struct nlist *lookup(char *s)
{
    struct nlist *np;

    for (np = hashtab[hash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
            return np;
    return NULL;
}

static struct nlist *install(char *name, unsigned code)
{
    struct nlist *np;
    unsigned hashval;

    if ((np = lookup(name)) == NULL)
    {
        np = (struct nlist *)malloc(sizeof(*np));

        if (np == NULL || (np->name = strdup(name)) == NULL)
            return NULL;

        hashval = hash(name);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    }

    np->code = code;
    return np;
}

int entity(char *name, int *len)
{
    int i, c, r;
    char *p, buf[64];
    struct nlist *np;

    for (i = 2, p = buf; i < 65; ++i)
    {
        c = *name++;

        if (c == ';' || isspace(c))  /* howcome 27/2/95: added test for space */
        {
            *p = '\0';
            
	    if (isspace(c))
		*len = i - 1;
	    else
		*len = i;

	    if ((r = atoi(buf + 1)) > 0) {
		return r;
	    } 
	    else {
		np = lookup(buf);
		if (np)
		    return np->code;
	    }

            break;
        }

        *p++ = c;
    }

    return 0;   /* signifies unknown entity name */
}

void InitEntities(void)
{
    struct entity *ep;
    
    ep = entities;

    for(;;)
    {
        install(ep->name, ep->code);

        if (strcmp(ep->name, "pd") == 0)
            break;

        ++ep;        
    }
} 


