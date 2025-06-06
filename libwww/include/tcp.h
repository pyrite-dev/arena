/*                                                      System dependencies in the W3 library
                                   SYSTEM DEPENDENCIES
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   System-system differences for TCP include files and macros. This file includes for each
   system the files necessary for network and file I/O.
   
   This module is a part of the W3C Reference Library.
   
  AUTHORS
  
  TBL                     Tim Berners-Lee, W3 project, CERN, <timbl@w3.org>
                         
  EvA                     Eelco van Asperen <evas@cs.few.eur.nl>
                         
  MA                      Marc Andreesen NCSA
                         
  MD                      Mark Donszelmann <duns@vxcern.cern.ch>
                         
  AT                      Aleksandar Totic <atotic@ncsa.uiuc.edu>
                         
  SCW                     Susan C. Weber <sweber@kyle.eitech.com>
                         
  HF                      Henrik Frystyk, <frystyk@w3.org>
                         
  CLB                     Charlie Brooks, <cbrooks@osf.org>
                         
  HISTORY:
  
  22 Feb 91               Written (TBL) as part of the WWW library.
                         
  16 Jan 92               PC code from (EvA)
                         
  22 Apr 93              Merged diffs bits from xmosaic release
                         
  29 Apr 93              Windows/NT code from (SCW)
                         
  29 Sep 93              Mar 96 CLB - changed SLEEP() macro for Windows/NT MSC compiler
                         added BOOLEAN_DEFINED macro to avoid duplicate definitions in
                         HUtils.h  changed netread() macros to support reading from
                         stdin,etc. as well  as sockets. (Required for linemode browser to
                         work).
                         
 */
#ifndef TCP_H
#define TCP_H
/*

Default values

   These values may be reset and altered by system-specific sections later on. There is
   also a bunch of defaults at the end and a section for ordinary BSD unix versions.
   
 */
#define SELECT                  /* Can handle >1 channel */
#define GOT_SYSTEM              /* Can call shell with string */
#define GOT_PASSWD              /* Can we use getpass() command */
#define TTY_IS_SELECTABLE       /* TTY can be passed to select() call */
/*

   If you want to use reentrant functions, for example gmtime_r then enable this flag
   
 */
#if 0
#define HT_REENTRANT
#endif
/*

   
   ___________________________________
   
                                 PLATFORM SPECIFIC STUFF
                                             
   Information below this line is specific for most platforms. See also General Stuff
   
Macintosh - Metrowerks Codewarrior 6

   Metrowerks Codewarrior is one development environment on the Mac. We are using GUSI
   (1.5.9) by Matthias Neeracher <neeri@iis.ee.ethz.ch> for our socket lib. You can find
   more information about the GUSI Library at
   http://err.ethz.ch/members/neeri/macintosh/gusiman/GUSI.html
   
       Compiles on PPC. Should compile on 68K.
   
       August 31, 1995 by Steven T. Roussey <sroussey@eng.uci.edu> (STR)
   
 */
#ifdef __MWERKS__
#include <gusi.h>
#include <dirent.h>
#include <errno.h>
#include <sys/errno.h>
#include <sioux.h>

#define INCLUDES_DONE
#define TCP_INCLUDES_DONE

#define GUSI                    /* Identifies changes made for GUSI */
#define NO_GETDOMAINNAME        /* STR */
#define NO_PASSWD               /* STR */

#define NO_GETWD
#define HAS_GETCWD
#define USE_DIRENT
#define NO_GROUPS
#define GOT_READ_DIR

#define NO_TIMEZONE             /* STR */
#define NO_GMTOFF
#define HAVE_STRERROR

#define STRUCT_DIRENT   struct dirent
#define d_ino           d_fileno        /* backward compatibility */

#define SLEEP(n)        GUSIDefaultSpin( SP_SLEEP, n/60)

#define MKDIR(a,b)      mkdir(a)
#endif
/*

Macintosh - MPW

   MPW is one development environment on the Mac.
   
   This entry was created by Aleksandar Totic (atotic@ncsa.uiuc.edu) this file is
   compatible with sockets package released by NCSA.  One major conflict is that this
   library redefines write/read/etc as macros.  In some of HTML code these macros get
   executed when they should not be. Such files should define NO_SOCKET_DEFS on top. This
   is a temporary hack.
   
 */
#ifdef applec                   /* MPW  */
#undef GOT_SYSTEM
#define DEBUG                   /* Can't put it on the CC command line */

#define NO_UNIX_IO              /* getuid() missing */
#define NO_GETPID               /* getpid() does not exist */
#define NO_GETWD                /* getwd() does not exist */

#define NETCLOSE s_close    /* Routine to close a TCP-IP socket */
#define NETREAD  s_read     /* Routine to read from a TCP-IP socket */
#define NETWRITE s_write    /* Routine to write to a TCP-IP socket */

#define _ANSI_SOURCE
#define GUI
#define LINEFEED 10
#define ANON_FTP_HOSTNAME
#ifndef NO_SOCKET_DEFS
#include <MacSockDefs.h>
#endif /* NO_SOCKET_DEFS */

#include <socket.ext.h>
#include <string.h>

#endif /* applec MPW */

#ifdef __QNX__
#define FD_SETSIZE	128	/* run-time specifiable limit */
#include <sys/select.h>
#endif

/*

Big Blue - the world of incompatibility

  IBM RS6000
  
   On the IBM RS-6000, AIX is almost Unix.
   
 */
#ifdef _AIX
#define AIX
#endif
#ifdef AIX
#define NO_ALTZONE
#define RLOGIN_USER
#define unix
#endif

/*    AIX 3.2
**    -------
*/

#ifdef _IBMR2
#define USE_DIRENT              /* sys V style directory open */
#endif
/*

  IBM VM-CMS, VM-XA MAINFRAMES
  
   MVS is compiled as for VM. MVS has no unix-style I/O.  The command line compile options
   seem to come across in lower case.
   
 */
#ifdef mvs
#define MVS
#endif

#ifdef MVS
#define VM
#endif

#ifdef NEWLIB
#pragma linkage(newlib,OS)      /* Enables recursive NEWLIB */
#endif

/*      VM doesn't have a built-in predefined token, so we cheat: */
#ifndef VM
#include <string.h>             /* For bzero etc - not  VM */
#endif

/*      Note:   All include file names must have 8 chars max (+".h")
**
**      Under VM, compile with "(DEF=VM,SHORT_NAMES,DEBUG)"
**
**      Under MVS, compile with "NOMAR DEF(MVS)" to get rid of 72 char margin
**        System include files TCPIP and COMMMAC neeed line number removal(!)
*/

#ifdef VM                       /* or MVS -- see above. */
#define GOT_PIPE                /* Of sorts */
#define NOT_ASCII               /* char type is not ASCII */
#define NO_UNIX_IO              /* Unix I/O routines are not supported */
#define NO_GETPID               /* getpid() does not exist */
#define NO_GETWD                /* getwd() does not exist */
#ifndef SHORT_NAMES
#define SHORT_NAMES             /* 8 character uniqueness for globals */
#endif
#include <manifest.h>
#include <bsdtypes.h>
#include <stdefs.h>
#include <socket.h>
#include <in.h>
#include <inet.h>
#include <netdb.h>
#include <errno.h>          /* independent */
extern char asciitoebcdic[], ebcdictoascii[];
#define TOASCII(c)   (c=='\n' ?  10  : ebcdictoascii[c])
#define FROMASCII(c) (c== 10  ? '\n' : asciitoebcdic[c])
#include <bsdtime.h>
#include <time.h>
#include <string.h>
#define INCLUDES_DONE
#define TCP_INCLUDES_DONE
#define SIMPLE_TELNET
#endif
/*

IBM-PC running MS-DOS with SunNFS for TCP/IP

   This code thanks to Eelco van Asperen <evas@cs.few.eur.nl>
   
 */
#ifdef PCNFS
#include <sys/types.h>
#include <string.h>
#include <errno.h>          /* independent */
#include <sys/time.h>       /* independent */
#include <sys/stat.h>
#include <fcntl.h>          /* In place of sys/param and sys/file */
#define INCLUDES_DONE
#define FD_SET(fd,pmask) (*(unsigned*)(pmask)) |=  (1<<(fd))
#define FD_CLR(fd,pmask) (*(unsigned*)(pmask)) &= ~(1<<(fd))
#define FD_ZERO(pmask)   (*(unsigned*)(pmask))=0
#define FD_ISSET(fd,pmask) (*(unsigned*)(pmask) & (1<<(fd)))
#define NO_GROUPS
#endif  /* PCNFS */
/*

PC running Windows (16-bit)

   Help provided by Eric Prud'hommeaux, Susan C. Weber <sweber@kyle.eitech.com>, Paul
   Hounslow <P.M.Hounslow@reading.ac.uk>, and a lot of other PC people.
   
 */
#if defined(_WINDOWS) || defined(_CONSOLE)
#define WWW_MSWINDOWS
#endif

#if defined(_WINDOWS) && !defined (_CONSOLE)
#define WWW_WIN_WINDOW
#endif

#if defined(_CONSOLE)
#define WWW_WIN_CONSOLE
#endif

#ifdef WWW_MSWINDOWS
#include <windows.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>

#define NETREAD(s,b,l)  recv((s),(b),(l),0)
#define NETWRITE(s,b,l) send((s),(b),(l),0)
#define NETCLOSE(s)     closesocket(s)
#define IOCTL(s,c,a)    ioctlsocket(s,c, (long *) a)

#define MKDIR(a,b)      mkdir((a))
#define REMOVE(a)       remove((a))
#define DEFAULT_SUFFIXES        "."

#ifdef TTY_IS_SELECTABLE
#undef TTY_IS_SELECTABLE
#endif

#include <io.h>
#include <string.h>
#include <process.h>
#include <time.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>

#ifndef _CONSOLE
#define NO_STDIO
#endif

#define NO_PASSWD
#define NO_ALTZONE
#define NO_GETWD
#define NO_FCNTL
#define HAS_GETCWD
#define NO_GETPASSWD

#define NO_GETDOMAINNAME
#define NO_RESOLV_CONF

#define BOOLEAN_DEFINED
#define INCLUDES_DONE
#define TCP_INCLUDES_DONE

#define SOCKET SOCKET                   /* WinSocks socket descriptor */
#define INVSOC INVALID_SOCKET           /* WinSocks invalid socket */

#define DESIRED_WINSOCK_VERSION 0x0101  /* we'd like winsock ver 1.1... */
#define MINIMUM_WINSOCK_VERSION 0x0101  /* ...but we'll take ver 1.1 :) */
/*

  FILE AND DIRECTORY ACCESS
  
   These next defintions are because the UNIX stuff is not supplied with BC4 (Paul
   Hounslow
   
 */
#define NO_UNIX_IO

typedef unsigned long mode_t;

#define _IFMT           0170000 /* type of file */
#define _IFDIR          0040000 /* directory */
#define _IFCHR          0020000 /* character special */
#define _IFBLK          0060000 /* block special */
#define _IFREG          0100000 /* regular */
#define _IFLNK          0120000 /* symbolic link */
#define _IFSOCK         0140000 /* socket */
#define _IFIFO          0010000 /* fifo */

#define S_ISUID         0004000 /* set user id on execution */
#define S_ISGID         0002000 /* set group id on execution */
#define S_ISVTX         0001000 /* save swapped text even after use */

#ifdef S_IREAD
#undef S_IREAD
#define S_IREAD         0000400 /* read permission, owner */
#endif

#ifdef S_IWRITE
#undef S_IWRITE
#define S_IWRITE        0000200 /* write permission, owner */
#endif

#ifdef S_IEXEC
#undef S_IEXEC
#define S_IEXEC         0000100 /* execute/search permission, owner */
#endif

#define S_ENFMT         0002000 /* enforcement-mode locking */

#ifdef S_IFMT
#undef S_IFMT
#define S_IFMT          _IFMT
#endif

#ifdef S_IDIR
#undef S_IDIR
#define S_IFDIR         _IFDIR
#endif

#ifdef S_IFCHR
#undef S_IFCHR
#define S_IFCHR         _IFCHR
#endif

#ifdef S_IBLK
#undef S_IBLK
#define S_IFBLK         _IFBLK
#endif

#ifdef S_IREG
#undef S_IREG
#define S_IFREG         _IFREG
#endif

#define S_IFLNK         _IFLNK

#ifdef S_IFIFO
#undef S_IFIFO
#define S_IFIFO         _IFIFO
#endif

#define S_IRWXU         0000700 /* rwx, owner */
#define         S_IRUSR 0000400 /* read permission, owner */
#define         S_IWUSR 0000200 /* write permission, owner */
#define         S_IXUSR 0000100 /* execute/search permission, owner */
#define S_IRWXG         0000070 /* rwx, group */
#define         S_IRGRP 0000040 /* read permission, group */
#define         S_IWGRP 0000020 /* write permission, grougroup */
#define         S_IXGRP 0000010 /* execute/search permission, group */
#define S_IRWXO         0000007 /* rwx, other */
#define         S_IROTH 0000004 /* read permission, other */
#define         S_IWOTH 0000002 /* write permission, other */
#define         S_IXOTH 0000001 /* execute/search permission, other */

#define S_ISREG(m)      (((m)&_IFMT) == _IFREG)
/*

  ERRNO AND RETURN CODES
  
   Winsock has its own errno codes and it returns them through WSAGetLastError(). However,
   it does also support BSD error codes, so we make a compromise. WSA definitions moved
   from _WIN32 ifdef by EGP
   
 */
#define socerrno WSAGetLastError()
#define ERRNO_DONE
/*

   Return code for socket functions. We can't use -1 as return value
   
 */
#define EWOULDBLOCK     WSAEWOULDBLOCK
#define EINPROGRESS     WSAEINPROGRESS
#define ECONNREFUSED    WSAECONNREFUSED
#define ETIMEDOUT       WSAETIMEDOUT
#define ENETUNREACH     WSAENETUNREACH
#define EHOSTUNREACH    WSAEHOSTUNREACH
#define EHOSTDOWN       WSAEHOSTDOWN
#define EISCONN         WSAEISCONN

/* Some compilers do only define WIN32 and NOT _WINDOWS */
#define NO_GROUPS

#ifdef _WIN32
#define MKDIR(a,b)      mkdir((a))     /* CLB NT has mkdir, but only one arg */
#define SLEEP(n)        Sleep((n)*1000)
#else
#define MKDIR(a,b)      _mkdir((a))    /* CLB NT has mkdir, but only one arg */
#endif /* WIN32 */

#endif /* WWW_MSWINDOWS */
/*

VAX/VMS

   Under VMS, there are many versions of TCP-IP. Define one if you do not use Digital's
   UCX product:
   
  UCX                     DEC's "Ultrix connection" (default)
                         
  WIN_TCP                 From Wollongong, now GEC software.
                         
  MULTINET                From SRI, now from TGV Inv.
                         
  DECNET                  Cern's TCP socket emulation over DECnet
                         
   The last three do not interfere with the unix i/o library, and so they need special
   calls to read, write and close sockets. In these cases the socket number is a VMS
   channel number, so we make the @@@ HORRIBLE @@@ assumption that a channel number will
   be greater than 10 but a unix file descriptor less than 10.  It works.
   
 */
#ifdef VMS
#include "HTVMSUtils.h"
#define CACHE_FILE_PREFIX       "SYS$LOGIN:Z_"
#define DEFAULT_SUFFIXES        "._"

#ifdef WIN_TCP
#define NETREAD(s,b,l)  ((s)>10 ? netread((s),(b),(l)) : read((s),(b),(l)))
#define NETWRITE(s,b,l) ((s)>10 ? netwrite((s),(b),(l)) : write((s),(b),(l)))
#define NETCLOSE(s)     ((s)>10 ? netclose(s) : close(s))
#endif /* WIN_TCP */

#ifdef MULTINET
#undef NETCLOSE
#undef NETREAD
#undef NETWRITE
#define NETREAD(s,b,l)  ((s)>10 ? socket_read((s),(b),(l)) : read((s),(b),(l)))
#define NETWRITE(s,b,l) ((s)>10 ? socket_write((s),(b),(l)) : \
                                write((s),(b),(l)))
#define NETCLOSE(s)     ((s)>10 ? socket_close(s) : close(s))
#define IOCTL(s,c,a)    socket_ioctl(s,c,a);
#endif /* MULTINET */

#ifdef DECNET
#define DNP_OBJ 80      /* This one doesn't look busy, but we must check */
                        /* That one was for decnet */
#undef SELECT           /* not supported */
#define NETREAD(s,b,l)  ((s)>10 ? recv((s),(b),(l),0) : read((s),(b),(l)))
#define NETWRITE(s,b,l) ((s)>10 ? send((s),(b),(l),0) : write((s),(b),(l)))
#define NETCLOSE(s)     ((s)>10 ? socket_close(s) : close(s))

#define NO_GETHOSTNAME                  /* Decnet doesn't have a name server */
#endif /* Decnet */

#define NO_RESOLV_CONF
#define NO_GETDOMAINNAME

/*      Certainly this works for UCX and Multinet; not tried for Wollongong
*/
#ifdef MULTINET
#include <time.h>
#ifdef __TIME_T
#define __TYPES
#define __TYPES_LOADED
#endif /* __TIME_T */
#include <multinet_root:[multinet.include.sys]types.h>
#include <multinet_root:[multinet.include]errno.h>
#ifdef __TYPES
#define __TIME_T
#endif /* __TYPE */
#ifdef __TIME_LOADED
#define __TIME
#endif /* __TIME_LOADED */
#include <multinet_root:[multinet.include.sys]time.h>
#else /* not MULTINET */
#include <types.h>
#include <errno.h>
#include <time.h>
#endif /* not MULTINET */

#include string

#ifndef STDIO_H
#include <stdio>
#define STDIO_H
#endif

#include file

#ifndef DECNET  /* Why is it used at all ? Types conflict with "types.h> */
#include unixio
#endif

#define INCLUDES_DONE

#ifdef MULTINET  /* Include from standard Multinet directories */
#include <multinet_root:[multinet.include.sys]socket.h>
#ifdef __TIME_LOADED  /* defined by sys$library:time.h */
#define __TIME  /* to avoid double definitions in next file */
#endif
#include <multinet_root:[multinet.include.netinet]in.h>
#include <multinet_root:[multinet.include.arpa]inet.h>
#include <multinet_root:[multinet.include]netdb.h>
#include <multinet_root:[multinet.include.sys]ioctl.h>

#else  /* not multinet */
#ifdef DECNET
#include <types.h>  /* for socket.h */
#include <socket.h>
#include <dn>
#include <dnetdb>

#else /* UCX or WIN */
#ifdef CADDR_T
#define __CADDR_T
#endif /* problem with xlib.h inclusion */
#include <socket.h>
#include <in.h>
#include <inet.h>
#include <netdb.h>
#include <ucx$inetdef.h>

#endif  /* not DECNET */
#endif  /* of Multinet or other TCP includes */

#define TCP_INCLUDES_DONE

#ifdef UCX
#define SIMPLE_TELNET
#endif
/*

   On VMS directory browsing is available through a separate copy of dirent.c. The
   definition of R_OK seem to be missing from the system include files...
   
 */
#define USE_DIRENT
#define GOT_READ_DIR
#include <dirent.h>
#define STRUCT_DIRENT struct dirent
#define R_OK 4
/*

   On VMS machines, the linker needs to be told to put global data sections into a data
   segment using these storage classes. (MarkDonszelmann)
   
 */
#ifdef VAXC
#define GLOBALDEF globaldef
#define GLOBALREF globalref
#endif /*  VAXC */
#endif  /* vms */
/*

   On non-VMS machines, the GLOBALDEF and GLOBALREF storage types default to normal C
   storage types.
   
 */
#ifndef GLOBALREF
#define GLOBALDEF
#define GLOBALREF extern
#endif
/*

   On non-VMS machines STAT should be stat...On VMS machines STAT is a function that
   converts directories and devices so that you can stat them.
   
 */
#ifdef VMS
typedef unsigned long mode_t;
#define HT_STAT         HTStat
#define HT_LSTAT        HTStat
#else
#define HT_STAT         stat
#define HT_LSTAT        lstat
#endif /* non VMS */
/*

  STRFTIME AND OTHER TIME STUFF
  
 */
#ifdef VMS
#ifndef DECC
#define NO_STRFTIME
#endif
#define NO_MKTIME
#define NO_TIMEGM
#define NO_GMTOFF
#define NO_TIMEZONE
#endif
/*

  DEFINITION OF ERRNO
  
 */
#ifdef VMS
#ifndef __DECC
extern int uerrno;      /* Deposit of error info (as per errno.h) */
extern volatile noshare int socket_errno; /* socket VMS error info
                                          (used for translation of vmserrno) */
extern volatile noshare int vmserrno;   /* Deposit of VMS error info */
extern volatile noshare int errno;  /* noshare to avoid PSECT conflict */
#define ERRNO_DONE
#endif /* not DECC */
#endif /* VMS */
/*

SCO ODT unix version

   (by Brian King)
   
 */
#ifdef sco
#include <grp.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <time.h>
#define USE_DIRENT
#define NEED_INITGROUPS
#define NO_GETDOMAINNAME
#endif
/*

BSDi BSD/386 Unix

   Specific stuff for BSDi BSD/386
   
 */
#ifdef bsdi
#define NO_GETDOMAINNAME
#endif
/*

MIPS unix

   Mips hack (bsd4.3/sysV mixture...)
   
 */
#ifdef Mips     /* Bruker */
typedef mode_t          int;
#define S_ENFMT         S_ISGID         /* record locking enforcement flag */
#define S_ISCHR(m)      ((m) & S_IFCHR)
#define S_ISBLK(m)      ((m) & S_IFBLK)
#define S_ISDIR(m)      (((m)& S_IFMT) == S_IFDIR)
#define S_ISREG(m)      (((m)& S_IFMT) == S_IFREG)
#define WEXITSTATUS(s)  (((s).w_status >> 8) & 0377)
#define NO_STRFTIME

/* Mips can't uppercase non-alpha */
#define TOLOWER(c) (isupper(c) ? tolower(c) : (c))
#define TOUPPER(c) (islower(c) ? toupper(c) : (c))
/*

  FILE PERMISSIONS
  
 */
#ifndef S_IRWXU
#define S_IRWXU 0000700
#define S_IRWXG 0000070
#define S_IRWXO 0000007
#define S_IRUSR 0000400
#define S_IWUSR 0000200
#define S_IXUSR 0000100
#define S_IRGRP 0000040
#define S_IWGRP 0000020
#define S_IXGRP 0000010
#define S_IROTH 0000004
#define S_IWOTH 0000002
#define S_IXOTH 0000001
#endif /* S_IRWXU */
#endif /* Mips */
/*

Linux

   Linux support (thanks to Rainer Klute)
   
 */
#ifdef linux
#include <unistd.h>
#include <limits.h>
#include <sys/fcntl.h>

#define NO_ALTZONE
#define NO_GETWD
#define HAS_GETCWD
#define FULL_TELNET
#endif /* linux */
/*

Solaris

   Solaris and other SVR4 based systems
   
 */
#if defined(__svr4__) || defined(_POSIX_SOURCE) || defined(__hpux) || defined(__sgi)

#if defined(__hpux)
#define NO_ALTZONE
#endif

#ifdef UTS4                                   /* UTS wants sys/types.h first */
#include <sys/types.h>
#endif

#include <unistd.h>

#ifdef UTS4
#include <sys/fcntl.h>
#define POSIXWAIT
#endif

#ifdef AIX                                                     /* Apple Unix */
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif
/*

   This is for NCR 3000 and Pyramid that also are SVR4 systems. Thanks to Alex Podlecki,
   <a.podlecki@att.com>
   
 */
#ifndef NGROUPS_MAX
#include <limits.h>
#endif

/* Pyramid and Mips can't uppercase non-alpha */
#ifdef pyramid
#define TOLOWER(c) (isupper(c) ? tolower(c) : (c))
#define TOUPPER(c) (islower(c) ? toupper(c) : (c))
#endif

/*

   getwd() is BSD. System V has getcwd()
   
 */
#define NO_GETWD
#define HAS_GETCWD

#endif /* Solaris and SVR4 */
/*

UTS 2.1 (BSD-like)

 */
#ifdef UTS2
#include <time.h>
#include <fcntl.h>

#ifndef R_OK
#define R_OK 4
#endif
#define NO_STRFTIME
#define WEXITSTATUS(x)  ((int)((x).w_T.w_Retcode))

#undef POSIXWAIT
#endif /* UTS2 */
/*

OSF/1

 */
#ifdef __osf__
#define USE_DIRENT
#define NO_TIMEZONE                                      /* OSF/1 has gmtoff */
#endif /* OSF1 AXP */
/*

Ultrix Decstation

 */
#ifdef decstation
#include <unistd.h>
#define NO_TIMEZONE                                     /* Ultrix has gmtoff */
#define RLOGIN_USER
#endif
/*

ISC 3.0

   by Lauren Weinstein <lauren@vortex.com>.
   
 */
#ifdef ISC3                     /* Lauren */
#define USE_DIRENT
#define GOT_READ_DIR
#include <sys/ipc.h>
#include <sys/dirent.h>
#define direct dirent
#include <sys/unistd.h>
#define d_namlen d_reclen
#include <sys/limits.h>
typedef int mode_t;

#define SIGSTP

#define POSIXWAIT
#define _POSIX_SOURCE

#include <sys/types.h>
#include <sys/wait.h>
#include <net/errno.h>

#define _SYSV3
#include <time.h>

#include <sys/fcntl.h>
#define S_ISDIR(m)      (((m) & 0170000) == 0040000)
#define S_ISCHR(m)      (((m) & 0170000) == 0020000)
#define S_ISBLK(m)      (((m) & 0170000) == 0060000)
#define S_ISREG(m)      (((m) & 0170000) == 0100000)
#define S_ISFIFO(m)     (((m) & 0170000) == 0010000)
#define S_ISLNK(m)      (((m) & 0170000) == 0120000)
#endif  /* ISC 3.0 */
/*

NeXT

   Next has a lot of strange changes in constants...
   
 */
#ifdef NeXT
#include <sys/types.h>
#include <sys/stat.h>

typedef unsigned short mode_t;

#ifndef S_ISDIR
#define S_ISDIR(m)     (m & S_IFDIR)
#define S_ISREG(m)     (m & S_IFREG)
#define S_ISCHR(m)     (m & S_IFCHR)
#define S_ISBLK(m)     (m & S_IFBLK)
#define S_ISLNK(m)     (m & S_IFLNK)
#define S_ISSOCK(m)    (m & S_IFSOCK)
#define S_ISFIFO(m)    (NO)
#endif

#ifndef WEXITSTATUS
#define WEXITSTATUS(s) (((s).w_status >> 8) & 0377)
#endif

#ifndef O_NONBLOCK
#define O_NONBLOCK      O_NDELAY
#endif

#define NO_TIMEZONE                                       /* next has gmtoff */
/*

  FILE PERMISSIONS FOR NEXT
  
 */
#ifndef S_IRWXU
#define S_IRWXU 0000700
#define S_IRWXG 0000070
#define S_IRWXO 0000007
#define S_IRUSR 0000400
#define S_IWUSR 0000200
#define S_IXUSR 0000100
#define S_IRGRP 0000040
#define S_IWGRP 0000020
#define S_IXGRP 0000010
#define S_IROTH 0000004
#define S_IWOTH 0000002
#define S_IXOTH 0000001
#endif /* S_IRWXU */
#endif /* NeXT */
/*

A/UX Apple UNIX

                 Kelly King, bhutab@apple.com and Jim Jagielski, jim@jagubox.gsfc.nasa.gov
                                                                                          
 */
#ifdef _AUX
#include <time.h>
#define WEXITSTATUS(s) (((s).w_status >> 8) & 0377)
#define NO_STRFTIME
#endif
/*

Sony News

   Thanks to Isao SEKI, seki@tama.prug.or.jp
   
 */
#ifdef sony_news
#include <sys/wait.h>
#define WEXITSTATUS(s) (((s).w_status >> 8) & 0377)
typedef int mode_t;
#endif
/*

Regular BSD unix versions

   These are a default unix where not already defined specifically.
   
 */
#ifndef INCLUDES_DONE

#include <sys/types.h>
#include <string.h>

#include <errno.h>          /* independent */
#include <sys/time.h>       /* independent */
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/file.h>       /* For open() etc */
#define INCLUDES_DONE
#endif  /* Normal includes */

#ifdef SUNOS
#include <unistd.h>
#define NO_TIMEZONE
#define NO_MKTIME
#undef HAVE_STRERROR
#endif

#ifdef NetBSD
#define NO_TIMEZONE
#define HAVE_STRERROR
#endif

#ifdef __FreeBSD__
#define NO_TIMEZONE
#define HAVE_STRERROR
#define NO_GETWD
#define HAS_GETCWD
#define FULL_TELNET
#endif

/*

  DIRECTORY READING STUFF - BSD OR SYS V
  
 */
#if defined(unix) || defined(__unix__)   /* If UNIX (thanks to Rainer Klute) */

#define GOT_PIPE                                        /* We do have a pipe */
#define GOT_READ_DIR         /* if directory reading functions are available */
#define GOT_OWNER                         /* If a file has owner permissions */

#include <pwd.h>
#include <grp.h>

#include <stdio.h>

#ifndef NeXT
#define USE_DIRENT                /* Try this all the time, Henrik May 29 94 */
#endif
#ifdef USE_DIRENT                                           /* sys v version */
#include <dirent.h>
#define STRUCT_DIRENT struct dirent
#else
#include <sys/dir.h>
#define STRUCT_DIRENT struct direct
#endif

#ifdef SOLARIS
#include <sys/fcntl.h>
#include <limits.h>
#endif
#ifndef FNDELAY
#define FNDELAY         O_NDELAY
#endif

#endif /* unix */
/*

   
   ___________________________________
   
                                      GENERAL STUFF
                                             
   Information below this line is general for most platforms.
   
   See also Platform Specific Stuff
   
  SLEEP SYSTEM CALL
  
   Some systems use milliseconds instead of seconds
   
 */
#ifndef SLEEP
#define SLEEP(n)        sleep(n)
#endif
/*

  SOCKS
  
   SOCKS modifications by Ian Dunkin <imd1707@ggr.co.uk>.
   
 */
#if defined(SOCKS) && !defined(RULE_FILE)
#define connect Rconnect
#define accept  Raccept
#define getsockname Rgetsockname
#define bind Rbind
#define listen Rlisten
#endif
/*

  NETWORK FAMILY
  
 */
#ifdef DECNET
typedef struct sockaddr_dn SockA;  /* See netdnet/dn.h or custom vms.h */
#else /* Internet */
typedef struct sockaddr_in SockA;  /* See netinet/in.h */
#endif
/*

  DEFAULT VALUES OF NETWORK ACCESS
  
 */
#ifndef NETCLOSE
#define NETCLOSE close          /* Routine to close a TCP-IP socket */
#endif

#ifndef NETREAD
#define NETREAD  read           /* Routine to read from a TCP-IP socket */
#endif

#ifndef NETWRITE
#define NETWRITE write          /* Routine to write to a TCP-IP socket */
#endif
/*

  INCLUDE FILES FOR TCP
  
 */
#ifndef TCP_INCLUDES_DONE
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>                 /* Must be after netinet/in.h */
#include <netdb.h>
#include <unistd.h>
#define TCP_INCLUDES_DONE
#endif
/*

  DEFINITION OF ERRNO AND RETURN CODE
  
   This is the definition of error codes and the corresponding string constants.  If we do
   not have the strerror function then try the error list table.
   
 */
#ifndef ERRNO_DONE
extern int errno;
#define socerrno errno
#endif

#ifndef HAVE_STRERROR                             /* Otherwise use the table */
extern char *sys_errlist[];
extern int sys_nerr;
#endif
/*

  DEFINITION OF SOCKET DESRCIPTORES
  
   This is necessary in order to support Windows NT...
   
 */
#ifndef SOCKET
#define SOCKET long long        /* Unix like socket descriptor */
#define INVSOC (-1)             /* Unix invalid socket */
#endif

#ifdef __svr4__
#define HT_BACKLOG 32            /* Number of pending connect requests (TCP) */
#else
#define HT_BACKLOG 5             /* Number of pending connect requests (TCP) */
#endif /* __svr4__ */
/*

  ADDITIONAL NETWORK VARIABLES FOR SOCKET, FILE ACCESS BITS
  
 */
#ifndef _WINSOCKAPI_
#define FD_READ         0x01
#define FD_WRITE        0x02
#define FD_OOB          0x04
#define FD_ACCEPT       0x08
#define FD_CONNECT      0x10
#define FD_CLOSE        0x20
#endif /* _WINSOCKAPI_ */
/*

  ROUGH ESTIMATE OF MAX PATH LENGTH
  
 */
#ifndef HT_MAX_PATH
#ifdef MAXPATHLEN
#define HT_MAX_PATH MAXPATHLEN
#else
#ifdef PATH_MAX
#define HT_MAX_PATH PATH_MAX
#else
#define HT_MAX_PATH 1024                        /* Any better ideas? */
#endif
#endif
#endif /* HT_MAX_PATH */
/*

  DEFINITION OF NGROUPS
  
 */
#ifdef NO_UNIX_IO
#define NO_GROUPS
#endif

#ifndef NO_GROUPS
#ifndef NGROUPS
#ifdef NGROUPS_MAX
#define NGROUPS NGROUPS_MAX
#else
#define NGROUPS 20                              /* Any better ideas? */
#endif
#endif
#endif
/*

  DEFINITION OF MAX DOMAIN NAME LENGTH
  
 */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64                       /* Any better ideas? */
#endif
/*

  FILE/DIRECTORY MANAGEMENT
  
 */
#ifndef MKDIR
#define MKDIR(a,b)      mkdir((a), (b))
#endif

#ifndef RMDIR
#define RMDIR(a)        rmdir((a))
#endif

#ifndef REMOVE
#define REMOVE(a)       unlink((a))
#endif

#ifndef DEFAULT_SUFFIXES
#define DEFAULT_SUFFIXES        ".,_"
#endif

#ifndef FCNTL
#define FCNTL(r,s,t)    fcntl((r),(s),(t))
#endif
/*

  MACROS FOR MANIPULATING MASKS FOR SELECT()
  
 */
#ifdef SELECT
#ifndef FD_SET
typedef unsigned int fd_set;
#define FD_SET(fd,pmask) (*(pmask)) |=  (1<<(fd))
#define FD_CLR(fd,pmask) (*(pmask)) &= ~(1<<(fd))
#define FD_ZERO(pmask)   (*(pmask))=0
#define FD_ISSET(fd,pmask) (*(pmask) & (1<<(fd)))
#endif  /* FD_SET */
#endif  /* SELECT */
/*

  MACROS FOR CONVERTING CHARACTERS
  
 */
#ifndef TOASCII
#define TOASCII(c) (c)
#define FROMASCII(c) (c)
#endif
/*

  CACHE FILE PREFIX
  
   This is something onto which we tag something meaningful to make a cache file name.
   used in HTWSRC.c at least. If it is nor defined at all, caching is turned off.
   
 */
#ifndef CACHE_FILE_PREFIX
#if defined(unix) || defined(__unix__)
#define CACHE_FILE_PREFIX  "/usr/wsrc/"
#endif
#endif
/*

  THREAD SAFE SETUP
  
   These are some constants setting the size of buffers used by thread safe versions of
   some system calls.
   
 */
#ifdef HT_REENTRANT
#define HOSTENT_MAX     128
#define CTIME_MAX       26
#endif
/*

 */
#endif
/*

   End of system-specific file */
