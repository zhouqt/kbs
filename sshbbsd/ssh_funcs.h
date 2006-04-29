#ifndef SSH_FUNCS_H_INCLUDED
#define SSH_FUNCS_H_INCLUDED
#ifdef SSHBBS

#include <packet.h>

unsigned long ssh_crc32(const unsigned char *buf,unsigned int len);
int ssh_write(int fd,const void *buf,size_t count);
ssize_t ssh_read(int fd,void *buf,size_t count);
int ssh_select(int n,fd_set *readfds,fd_set *writefds,fd_set *exceptfds,struct timeval *timeout);
int ssh_init(void);
int bbs_entry(void);
#ifndef HAVE_IPV6_SMTH
int check_IP_lists(unsigned int IP2);
#else /* HAVE_IPV6_SMTH */
int check_IP_lists(struct in6_addr sip);
#endif /* HAVE_IPV6_SMTH */


#endif /* SSHBBS */
#endif /* SSH_FUNCS_H_INCLUDED */
