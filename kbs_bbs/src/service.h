#ifndef BBS_SERVICE_H
#define BBS_SERVICE_H

#ifdef CYGWIN
#define SMTH_API __declspec(dllimport)
#else
#define SMTH_API extern
#endif

#endif /* BBS_SERVICE_H */
