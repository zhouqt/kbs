/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* Firebird BBS 2.0 CONFIGURATION VARIABLES */

/* 
   Define this to enable wide character support for Chinese or other 
   languages not using the ANSI character set. Comment out if this isn't
   needed. 
*/
#define BIT8 

/* 
   You may define a default board here, already selected when a user logs in.
   If this is set to the empty string, the bbs will require users to (S)elect
   a board before reading or posting. Don't comment this out. 
*/
/*#define DEFAULTBOARD    "sysop"*/ /* Leeward 98.05.16 */
/*#define DEFAULTBOARD    "test" */

/* 
   You get this many chances to give a valid userid/password combination
   before the bbs squawks at you and closes the connection. 
   Don't comment out. 
*/
#define LOGINATTEMPTS 3

/* 
   Turn this on to allow users to create their own accounts by typing 'new'
   at the "Enter userid:" prompt. Comment out to restrict access to accounts
   created by the Sysop (see important note in README.install). 
*/
#define LOGINASNEW 1 

/* 
   Defined means user may bbslog as as many times as they wish, simultaneously.
   On a larger bbs with many window-users you probably want this off.
   Comment out to disable.
#define MULTI_LOGINS 1
*/

/* 
Comment this out to disable the Voting stuff. Dunno why, it's not a resource
hog or anything, but if you don't want it...
*/
#define VOTE    1

/* 
   Define this if you want the Internet Post/Mail Forwarding features
   enabled. You MUST have the sendmail(8) program on your system and
   a cron job to run the bbs mail queue every so often. The bbs does not
   invoke sendmail, it simply creates the queue and data files for it. 
*/
#define INTERNET_EMAIL 1

#ifdef INTERNET_EMAIL
/* If you defined INTERNET_EMAIL, you have the option of telling the bbs
   an address where failed forwarded messages can go to (people will 
   invariably set their addresses wrong!). If this is commented out bounces
   will go to the bbs mailbox in the usual mail spool. 
*/
#define ERRORS_TO "root"
#endif

/* 
   Define DOTIMEOUT to set a timer to bbslog out users who sit idle on the system.
   Then decide how long to let them stay: MONITOR_TIMEOUT is the time in
   seconds a user can sit idle in Monitor mode; IDLE_TIMEOUT applies to all
   other modes. 
*/
#define DOTIMEOUT 1

/* 
   These are moot if DOTIMEOUT is commented; leave them defined anyway. 
*/
#define IDLE_TIMEOUT    (60*20) 
#define MONITOR_TIMEOUT (60*20) 

/* 
   This defines the set of characters disallowed in chat id's. These
   characters get translated to underscores (_) when someone tries to use
   them. At the very least you should disallow spaces and '*'.
*/ 

#define BADCIDCHARS " *"

/* 
   By default, users with post permission can reply to a post right after
   they read it. This can lead to lots of silly replies, hence the reply
   feature is a little controversial. Uncomment this to disable Reply. 
*/
/*
#define NOREPLY 1 
*/
/* 
   The Eagle's Nest has problems with apparently random corruption of the
   board and mail directories. Other boards don't have this problem. Anyway,
   this ifdef enables some code that decreases this problem. you probably 
   won't need it, but... 
*/
/* #define POSTBUG      1 */

/* 
   Define this if you are going to install an Internet Relay Chat client
   and/or server. Either the one with this distribution or a newer one. 
#define IRC   1 
*/
#undef IRC

/***************************************************************************
BBSDOORS and MMMM are contributed by Bill Schwartz <schwartz@augsburg.edu>.
These were added right before this (2.0) release and are pretty much verbatim 
as contributed; also you'll have to install the programs and directories for
these two options by hand. Instructions are in README.install.

(I guess my point is...these are new to me so I don't guarantee them :)
****************************************************************************/

/*
   Define this if you want to set up doors to other systems via the BBSNet
   script. You'll need to customize and install the bbsnet script, and copy 
   more programs into the bbs subtree. See README.install for details.
*/
#define BBSDOORS        1 

/****************************************************************************/

#define MAXSIGLINES    6 /* max. # of lines appended for post signature */
#define MAXQUERYLINES 16 /* max. # of lines shown by the Query function */

/* Once you set this, do not change it unless you are restarting your
   bbs from scratch! The PASSWDS file will not work if this is changed.
   If commented out, no real name/address info can be kept in the passwd
   file. Pretty useless to have this if LOGINASNEW is defined. */

/* 
Define this to show real names in the Main Menu user list. 
*/
/* #define ACTS_REALNAMES       1 */

/* 
Define this to show real names in post headers. 
*/
/* #define POSTS_REALNAMES      1 */

/* 
Define this to show real names in mail messages. 
*/
/* #define MAIL_REALNAMES       1 */

/* 
Define this for Query to show real names. 
*/
/* #define QUERY_REALNAMES      1 */
/*如果 1 则板主砍 user 文章 , user post 数会减一，反之不减*/
#define BMDEL_DECREASE 1
/*Define this for Use Notepad.*/
#define USE_NOTEPAD      1

#define MAIL_LIMIT      50   /* Leeward: 25->50 due to new system:) */

/*#define LEEWARD_X_FILTER  Leeward: define this macro to disable filters */
#define LEEWARD_X_FILTER

