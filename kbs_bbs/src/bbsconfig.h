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

#ifndef BBSCONFIG_H_INCLUDED
#define BBSCONFIG_H_INCLUDED


/* Firebird BBS 2.0 CONFIGURATION VARIABLES */

/* 
   Define this to enable wide character support for Chinese or other 
   languages not using the ANSI character set. Comment out if this isn't
   needed. 
*/
#define BIT8 

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
   Define this if you want the Internet Post/Mail Forwarding features
   enabled. You MUST have the sendmail(8) program on your system and
   a cron job to run the bbs mail queue every so often. The bbs does not
   invoke sendmail, it simply creates the queue and data files for it. 
*/
#define INTERNET_EMAIL 1

/* 
   Define this if you are going to install an Internet Relay Chat client
   and/or server. Either the one with this distribution or a newer one. 
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
#define BBSDOORS 1 

/****************************************************************************/

#define MAXSIGLINES    6 /* max. # of lines appended for post signature */
#define MAXQUERYLINES 16 /* max. # of lines shown by the Query function */

/* Once you set this, do not change it unless you are restarting your
   bbs from scratch! The PASSWDS file will not work if this is changed.
   If commented out, no real name/address info can be kept in the passwd
   file. Pretty useless to have this if LOGINASNEW is defined. */

/*Define this for Use Notepad.*/
#define USE_NOTEPAD 1

#endif /* BBSCONFIG_H_INCLUDED */

