#include "bbs.h"
#define SWAP_WORD(x) ((((x)&0xff)<<8) | ( ((x)&0xff00)>>8))
#define SWAP_DWORD(x) (( SWAP_WORD(((x)&0xffff0000)>>16)) | ( SWAP_WORD((x)&0xffff)<<16) )
#define DO_SWAP_DWORD(x) x=SWAP_DWORD(x)

main()
{
    int filesize, fdr, i;
    struct userec *ptr;
    struct boardheader *boards;

    chdir(BBSHOME);
    switch (safe_mmapfile(".PASSWDS", O_RDWR | O_CREAT, PROT_READ | PROT_WRITE, MAP_SHARED, (void **) &ptr, &filesize, &fdr)) {
    case 0:
	return;
    case 1:
	for (i = 0; i < MAXUSERS; i++) {
/*
			char buf[1024];
			if (ptr[i].userid[0]) {
				sethomepath(buf,ptr[i].userid);
				mkdir(buf,0755);
				setmailpath(buf,ptr[i].userid);
				mkdir(buf,0755);
			}
			continue;
*/
	    DO_SWAP_DWORD(ptr[i].firstlogin);
	    DO_SWAP_DWORD(ptr[i].numlogins);
	    DO_SWAP_DWORD(ptr[i].numposts);
	    DO_SWAP_DWORD(ptr[i].userlevel);
	    DO_SWAP_DWORD(ptr[i].lastlogin);
	    DO_SWAP_DWORD(ptr[i].stay);
	    DO_SWAP_DWORD(ptr[i].signature);
	    DO_SWAP_DWORD(ptr[i].userdefine);
	    DO_SWAP_DWORD(ptr[i].notedate);
	    DO_SWAP_DWORD(ptr[i].noteline);
	    DO_SWAP_DWORD(ptr[i].notemode);
	    DO_SWAP_DWORD(ptr[i].exittime);
	    DO_SWAP_DWORD(ptr[i].unuse2);
	}
    }
    end_mmapfile((void *) ptr, filesize, fdr);
    switch (safe_mmapfile(".BOARDS", O_RDWR | O_CREAT, PROT_READ | PROT_WRITE, MAP_SHARED, (void **) &boards, &filesize, &fdr)) {
    case 0:
	return;
    case 1:
	for (i = 0; i < MAXBOARD; i++) {
/*
			if (boards[i].filename[0]) {
			char buf[1024];
			setbpath(buf,boards[i].filename);
			mkdir(buf,0755);
			};
*/
	    DO_SWAP_DWORD(boards[i].level);
	}
    }
    end_mmapfile((void *) boards, filesize, fdr);
    return;
}
