#ifndef __FLOCK_H
#define __FLOCK_H

#ifndef HAVE_FLOCK
/* Operations for the `flock' call.  */
#define LOCK_SH 1       /* Shared lock.  */
#define LOCK_EX 2       /* Exclusive lock.  */
#define LOCK_UN 8       /* Unlock.  */

int flock(int fd, int operation);

#endif /* !HAVE_FLOCK */

#endif /* !__FLOCK_H */
