/* define variable used by whole project */

extern struct userec lookupuser;
extern struct UTMPFILE *utmpshm;
extern struct UCACHE *uidshm;
extern int utmpent;

extern struct friends_info *topfriend;

struct UTMP_POS {
	int key;
	int curpos;
};

extern int     nf;

