#include "service.h"
#include "bbs.h"
#include <sys/times.h>

static int on=-1;
int a[21][12]={
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,0,0,0,0,0,0,0,0,0,0,8,
8,8,8,8,8,8,8,8,8,8,8,8};

unsigned char userid[30]="unknown.", userip[60]="unknown.";
int dx[7][4][4], dy[7][4][4];
int d[7][4][4]={
0,1,4,5, 0,1,4,5, 0,1,4,5, 0,1,4,5,
4,5,6,7, 1,5,9,13, 4,5,6,7, 1,5,9,13,
0,1,5,6, 1,4,5,8, 0,1,5,6, 1,4,5,8,
1,2,4,5, 0,4,5,9, 1,2,4,5, 0,4,5,9, 
0,1,2,4, 0,1,5,9, 2,4,5,6, 0,4,8,9,
0,1,2,6, 1,5,8,9, 0,4,5,6, 0,1,4,8,
0,1,2,5, 1,4,5,9, 1,4,5,6, 0,4,5,8};

int k,n,y,x,e;
int newk=0;
int lines=0;
int delay, level;
char topID[20][20],topFROM[20][32];
int topT[20];
 
int getch()
{
    int c,d,e;
	int ret;

	static int *retbuf;
	static int retlen=0;

	extern int keymem_total;
	extern struct key_struct *keymem;

	if(retlen > 0){
		ret = *retbuf;
		retbuf ++;
		retlen --;

		return ret;
	}

    c=getch0();
    if(c==3||c==4||c==-1) return -1;
	if(c==Ctrl('C')) return c;

    if(c!=KEY_ESC) ret=c;
	else{
    	d=getch0();
    	e=getch0();
    	if(d==0) ret=c;
    	else if(e=='A') ret=KEY_UP;
    	else if(e=='B') ret=KEY_DOWN;
    	else if(e=='C') ret=KEY_RIGHT;
    	else if(e=='D') ret=KEY_LEFT;
	}

    if(keymem_total) {
        int i,j,p;
        for(i=0;i<keymem_total;i++) {
            p=!keymem[i].status[0];
            if(keymem[i].status[0]==-1) continue;
            j=0;
            while(keymem[i].status[j]&&j<10) {
                if(keymem[i].status[j]==TETRIS) p=1;
                j++;
            }
            if(p&&ret==keymem[i].key) {
                j=0;
                while(keymem[i].mapped[j]&&j<10) j++;
                if(j==0) continue;
                ret = keymem[i].mapped[0];
                retlen = j-1;
                retbuf = keymem[i].mapped+1;
                break;
            }
        }
	}

    return ret;
}

int getch0()
{
    int c,d,e,n;
    char ch; 
    fd_set rfds;
    struct timeval tv;
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 50000;
    if(select(1, &rfds, NULL, NULL, &tv)) {
    	if(read(0, &ch, 1)<=0) exit(-1);
        return ch;
    }
    else
        return 0; 
}

int color(int c)
{
  static lastc=-1;
  char tmp[200];

  if(c==lastc)return;
  lastc=c;
  if(c==4) c=12;
  sprintf(tmp,"\033[%d;%dm",c/8,c%8+30);
  prints(tmp);
}

int clear2()
{
   int y;
   clear();
   move(3,0);
   prints("                            \033[1;33m┌─┐     \n");
   prints("                            │Ｔ│     \n");
   prints("                            └─┘     \n");
   prints("                            \033[1;34m┌─┐     \n");
   prints("                            │Ｅ│     \n");
   prints("                            └─┘     \n");
   prints("                            \033[1;33m┌─┐     \n");
   prints("                            │Ｔ│     \n");
   prints("                            └─┘     \n");
   prints("                            \033[1;35m┌─┐     \n");
   prints("                            │Ｒ│     \n");
   prints("                            └─┘     \n");
   prints("                            \033[1;31m┌─┐     \n");
   prints("                            │Ｉ│     \n");
   prints("                            └─┘     \n");
   prints("                            \033[1;32m┌─┐     \n");
   prints("                            │Ｓ│     \n");
   prints("                            └─┘\033[m     \n");

   prints("\033[1;33m祝你玩的愉快! \033[m按 '\033[1;32mCtrl+C\033[m'退出.");
}

int sh2()
{ 
   static int oy=-1, ox=-1, ok=-1;

   if(oy==y && ox==x && ok==k && on==n) return;
   sh(oy, ox, ok, on, 0);
   oy=y; ox=x; ok=k; on=n;
   sh(oy,ox,ok,on,ok+1);
   refresh();
}

int sh(int y, int x, int k, int n, int c)
{
   if(n==-1) return;
   for(e=0;e<=3;e++)
   {
     move(y+dy[k][n][e],2*(x+dx[k][n][e]));
     color(c);
     if(c)prints("■");else prints("  ");
   }
   move(0,0);
}

int show0()
{
  int y,x;
  for(y=0;y<=20;y++)
  {
    move(y,0);
    for(x=0;x<=11;x++)
    {
      color(a[y][x]);
      if(a[y][x])
        prints("■");
      else
        prints("  ");
    }
  }
  refresh();
}

int tetris_main()
{
  int n;
  int oldmode;

  strcpy(userid,getCurrentUser()->userid);
  strcpy(userip,"unknown.");

#ifdef SSHBBS
  return 0;
#endif

  modify_user_mode(TETRIS);
  intr();
  start();
  quit();

  return 0;
}

quit()
{
  color(7);
  prints("\033[H\033[J欢迎常来, 再见!\n");
  refresh();
  return 0;
}

init_data()
{
  for(k=0;k<=6;k++)
  for(n=0;n<=3;n++)
  for(e=0;e<=3;e++)
  {
     dx[k][n][e]=d[k][n][e]%4;
     dy[k][n][e]=d[k][n][e]/4;
  }
  for(y=0;y<=19;y++)
  for(x=1;x<=10;x++)
  {
     a[y][x]=0;
  }
  srand(time(0));
  newk=rand()%7;
  level=0;
  delay=200;
  lines=0;
}

int crash2(int x, int y, int k, int n)
{
  for(e=0;e<=3;e++)
  if(a[y+dy[k][n][e]][x+dx[k][n][e]])return 1;
  return 0;
}

start()
{
  int c,t,first;
  struct tms faint;
  win_showrec();
  while(1)
  {
    init_data();
    clear2();
    show0();
    first=1;
    while(1)
    {
      k=newk;
      newk=rand()%7;
      n=0;
      color(0);
      move(0,25);prints("                ");
      move(1,25);prints("                ");
      sh(0,14,newk,0,newk+1);
      n=0;
      x=3;y=0;
      sh2();
      if(first) {pressanykey();first=0;}
      if(crash2(x,y,k,n)) 
      {
         win_checkrec(lines);
         break;
      }
      t=times(&faint);
      while(1)
      {
    	//c = igetkey();
		c = getch();
		if (c==-1) return 0;
        if (c==27) pressanykey();
    	if (c == Ctrl('D') || c == Ctrl('C') ) return 0;
        if (c==KEY_LEFT||c=='a'||c=='A') if(!crash2(x-1,y,k,n)){x--;sh2();}
        if (c==KEY_RIGHT||c=='s'||c=='S') if(!crash2(x+1,y,k,n)){x++;sh2();}
        if (c=='b'||c=='B'||c=='\n'||c=='\r') if(!crash2(x,y,k,(n+1)%4)){n=(n+1)%4;sh2();}
        if (c=='h'||c=='H'||c==KEY_UP) if(!crash2(x,y,k,(n+3)%4)){n=(n+3)%4;sh2();}  
        if (c=='J'||c=='j') if(!crash2(x,y,k,(n+2)%4)){n=(n+2)%4;sh2();}
        if (c==' ') {while(!crash2(x,y+1,k,n))y++;sh2();down();break;}
        if (times(&faint)-t>delay||c==KEY_DOWN||c=='z'||c=='Z')
        {
          t=times(&faint);
          if(crash2(x,y+1,k,n)){down();break;} else {y++;sh2();}  
        }
      }
    }
  }
}

down()
{
   for(e=0;e<=3;e++)
   a[y+dy[k][n][e]][x+dx[k][n][e]]=k+1;
   checklines();
   on=-1;
}

checklines()
{
   int y1,x1;
   int y2;
   int s;
   s=0;   
   for(y1=0;y1<=19;y1++)
   {
      for(x1=1;x1<=10;x1++)
      if(a[y1][x1]==0)break;
      if(x1<=10) continue;
      s=1;
	  move(23,0);
      prints("Lines =\033[32m%3d", lines+1);
      if(lines==0) prints("                         ");
      for(y2=y1;y2>=1;y2--)
      for(x1=1;x1<=10;x1++)
        a[y2][x1]=a[y2-1][x1];  
      for(x1=1;x1<=10;x1++) 
        a[0][x1]=0;
      if((++lines)%30==0) 
        {delay*=.9; level++; bell();move(23,15);prints("Level =\033[32m%3d", level);} 
   }
   if(s) {show0();}
}

intr()
{
  clear();
  prints("欢迎光临\033[1;32m%s!\033[1m \033[1;35m%s.\033[m 您是本游戏的第 \033[1;33m%d\033[m 位访问者。\r\n\r\n",BBS_FULL_NAME, userid, count());
  prints("键盘设置: \r\n");
  prints("左: '\033[1;32ma\033[m', '\033[1;32m←\033[m';\r\n");
  prints("右: '\033[1;32ms\033[m', '\033[1;32m→\033[m';\r\n");
  prints("下: '\033[1;32mz\033[m', '\033[1;32m↓\033[m';\r\n");
  prints("顺时针转动: '\033[1;32mb\033[m', <\033[1;32mCR\033[m>; 逆时针转动: '\033[1;32mh\033[m', '\033[1;32m↑\033[m'; 180度转动: '\033[1;32mj\033[m';\r\n");
  prints("快降: ' ', 暂停: <\033[1;32mESC\033[m>.\r\n");
  prints("退出: '\033[1;32m^C\033[m', '\033[1;32m^D\033[m'.\r\n\r\n");
  prints("每消 \033[1;33m30\033[m 行升一级. \r\n"); 
  pressanykey();
  clear();
}

int count()
{
  int c;
  FILE *fp;
  fp=fopen("tetris.count","r");
  if(fp==NULL) {system("echo 1 > tetris.count"); return 1;}
  fscanf(fp,"%d",&c);
  fclose(fp);
  fp=fopen("tetris.count","w");
  fprintf(fp,"%d",c+1);
  fclose(fp);
  return c+1;
}

int win_loadrec()
{
  FILE *fp;
  int n;
  for(n=0;n<=19;n++)
  {
    strcpy(topID[n],"unknown.");
    topT[n]=0;
    strcpy(topFROM[n],"unknown.");
  }
  fp=fopen("tetris.rec","r");
  if(fp==NULL){win_saverec();return;}
  for(n=0;n<=19;n++)
    fscanf(fp,"%s %d %s\n",topID[n],&topT[n],&topFROM[n]);
  fclose(fp);
}

int win_saverec()
{
  FILE *fp;
  int n;
  fp=fopen("tetris.rec","w");
  for(n=0;n<=19;n++)
    {
      fprintf(fp,"%s %d %s\n",topID[n],topT[n],topFROM[n]);
    }
  fclose(fp);
}

int win_showrec()
{
  int n;
  char tmp[200];

  win_loadrec();
  clear();
  prints("\033[44;37m                         -       TETRIS 排行榜-                                 \r\n\033[m");
  prints("\033[41m No.          ID        LINES                         FROM                      \033[m\n\r");
  for(n=0;n<=19;n++)
  {
    sprintf(tmp, "\033[1;37m%3d\033[32m%13s\033[0;37m%12d\033[m%29s\n\r",n+1,topID[n],topT[n]
         ,topFROM[n]);
    prints(tmp);
  }
  prints("\033[41m                                                                               \033[m\n\r");
  pressanykey();
}

int win_checkrec(int dt)
{
  char id[30];
  int n;
  win_loadrec();
  strcpy(id,userid);
  for(n=0;n<=19;n++)
    if(!strcmp(topID[n],id))
    {
      if(dt>topT[n])
      {
        topT[n]=dt;
        strcpy(topFROM[n],userip);
        win_sort();
        win_saverec();
        win_showrec();
      }
      return;
    }
  if(dt>topT[19])
  {
    strcpy(topID[19],id);
    topT[19]=dt;
    strcpy(topFROM[19],userip);
    win_sort();
    win_saverec();
    win_showrec();
    return;
  }
}

int win_sort()
{
  int n,n2,tmp;
  char tmpID[30];
  clear();
  prints("祝贺! 您刷新了自己的纪录!\r\n");
  pressanykey();
  for(n=0;n<=18;n++)
  for(n2=n+1;n2<=19;n2++)
    if(topT[n]<topT[n2])
    {
      tmp=topT[n];topT[n]=topT[n2];topT[n2]=tmp;
      strcpy(tmpID,topID[n]);strcpy(topID[n],topID[n2]);
      strcpy(topID[n2],tmpID);
      strcpy(tmpID,topFROM[n]);strcpy(topFROM[n],topFROM[n2]);
      strcpy(topFROM[n2],tmpID);
    }
}
