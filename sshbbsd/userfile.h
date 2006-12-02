/*

userfile.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Wed Jan 24 19:53:02 1996 ylo

Functions for reading files as the real user from a program running as root.

This works by forking a separate process to do the reading.

*/

#ifndef USERFILE_H
#define USERFILE_H

typedef struct UserFile *UserFile;

/* Initializes reading as a user.  Before calling this, I/O may only be
   performed as the user that is running the current program (current
   effective uid).  SIGPIPE should be set to ignored before this call.
   The cleanup callback will be called in the child before switching to the
   user's uid.  The callback may be NULL. */
void userfile_init(const char *username, uid_t uid, gid_t gid, void (*cleanup_callback) (void *), void *context);

/* Stops reading files as an ordinary user.  It is not an error to call this
   even if userfile_init has not been called. */
void userfile_uninit(void);

/* Closes any pipes the userfile might have open.  This should be called after
   every fork. */
void userfile_close_pipes(void);

/* Opens a file using the given uid.  The uid must be either the current
   effective uid (in which case userfile_init need not have been called) or
   the uid passed to a previous call to userfile_init.  Returns a pointer
   to a structure, or NULL if an error occurred.  The flags and mode arguments
   are identical to open(). */
UserFile userfile_open(uid_t uid, const char *path, int flags, mode_t mode);

/* Closes the userfile handle.  Returns >= 0 on success, and < 0 on error. */
int userfile_close(UserFile f);

/* Returns the next character from the file (as an unsigned integer) or -1
   if an error is encountered. */
int userfile_getc(UserFile f);

/* Reads data from the file.  Returns as much data as is the buffer
   size, unless end of file is encountered.  Returns the number of bytes
   read, 0 on EOF, and -1 on error. */
int userfile_read(UserFile f, void *buf, unsigned int len);

/* Writes data to the file.  Writes all data, unless an error is encountered.
   Returns the number of bytes actually written; -1 indicates error. */
int userfile_write(UserFile f, const void *buf, unsigned int len);

/* Reads a line from the file.  The line will be null-terminated, and
   will include the newline.  Returns a pointer to the given buffer,
   or NULL if no more data was available.  If a line is too long,
   reads as much as the buffer can accommodate (and null-terminates
   it).  If the last line of the file does not terminate with a
   newline, returns the line, null-terminated, but without a
   newline. */
char *userfile_gets(char *buf, unsigned int size, UserFile f);

/* Performs lseek() on the given file. */
off_t userfile_lseek(UserFile uf, off_t offset, int whence);

/* Creates a directory using the given uid. */
int userfile_mkdir(uid_t uid, const char *path, mode_t mode);

/* Performs stat() using the given uid. */
int userfile_stat(uid_t uid, const char *path, struct stat *st);

/* Performs remove() using the given uid. */
int userfile_remove(uid_t uid, const char *path);

/* Performs popen() on the given uid; returns a file from where the output
   of the command can be read (type == "r") or to where data can be written
   (type == "w"). */
UserFile userfile_popen(uid_t uid, const char *command, const char *type);

/* Performs pclose() on the given uid.  Returns <0 if an error occurs. */
int userfile_pclose(UserFile uf);

/* Check owner and permissions of a given file/directory.
   Permissions ----w--w- must not exist and owner must be either
   pw->pw_uid or root. Return value: 0 = not ok, 1 = ok */
int userfile_check_owner_permissions(struct passwd *pw, const char *path);

/* Encapsulate a normal file descriptor inside a struct UserFile */
UserFile userfile_encapsulate_fd(int fd);

/* Get sun des 1 magic phrase, return NULL if not found */
char *userfile_get_des_1_magic_phrase(uid_t uid);

#endif                          /* USERFILE_H */
