/* Written By Leeward@etang.com, Inc. 1999 */

#ifndef __CACHE_H_
#define __CACHE_H_


#include <stdio.h>
#include <string.h>
#include <errno.h>


#define MAX_CA_PATH_LEN 1024


int ca_mkdir(const char *path, int mode);
int ca_rmdir(const char *path);


int ca_badpath(const char *path);


FILE *ca_fopen(const char *path, const char *mode);
#define ca_fclose(fp) fclose(fp);


int ca_expire(const char *URL);


#endif /* __CACHE_H_ */
