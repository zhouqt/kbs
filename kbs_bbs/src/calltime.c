#include "bbs.h"

struct clock_struct * clock_data = NULL;
int clock_total = 0;

extern time_t calltime;
extern char calltimememo[];

static int default_clock_data()
{
	clock_total=1;
	clock_data->type=1;
	clock_data->clock_time=0;
	clock_data->memo[0]='\0';
	return save_clock_data();
}

static int init_clock_data()
{
	FILE *fp;
	char fname[STRLEN];
	struct stat st;

	sethomefile(fname,currentuser->userid,"clock.data");
	stat(fname,&st);
	if((fp=fopen(fname,"rb"))==NULL){
		return default_clock_data();
	}
	if(st.st_size > MAX_CLOCK_DEF * sizeof(struct clock_struct) ){
		fclose(fp);
		return default_clock_data();
	}
	clock_total = st.st_size/sizeof(struct clock_struct);
	if(clock_total < 1){
		fclose(fp);
		return default_clock_data();
	}
	fread(clock_data,st.st_size,1,fp);
	fclose(fp);
	return 1;
}

static int save_clock_data()
{
	FILE *fp;
	char fname[STRLEN];
	struct stat st;

	sethomefile(fname,currentuser->userid,"clock.data");
	fp=fopen(fname,"wb");
	if(fp==NULL) return 0;
	fwrite(clock_data,clock_total*sizeof(struct clock_struct),1,fp);
	fclose(fp);
	truncate(fname,clock_total*sizeof(struct clock_struct));
	return 1;
}

static int add_clock_data(struct clock_struct * ck)
{
	init_clock_data();
	if( clock_total >= MAX_CLOCK_DEF )
		return 0;
	memcpy(clock_data + clock_total, ck, sizeof(struct clock_struct));
	clock_total ++;
	save_clock_data();
	return 1;
}

static int del_clock_data(int num)
{
	int i;

	if(num >= clock_total) return 0;

	for(i=num;i<clock_total-1;i++){
		memcpy(clock_data+i,clock_data+i+1,sizeof(struct clock_struct));
	}
	clock_total --;
	return 1;
}

static int del_clock(int num)
{
	init_clock_data();
	if(del_clock_data(num)){
		save_clock_data();
		calc_calltime(2);
		return SHOW_DIRCHANGE;
	}
	return SHOW_CONTINUE;
}

static time_t get_realcalltime(struct clock_struct * ck)
{

	if( ck->type == CLOCK_TYPE_DAY ){
		struct tm newtm;
		struct tm ltime;
		struct tm nowtm;
		time_t now;
		time_t newtmt;

		now = time(0);

		memcpy(&ltime, localtime( & (ck->clock_time) ), sizeof(struct tm));
		memcpy(&nowtm, localtime( & now ), sizeof(struct tm));

		newtm.tm_isdst = nowtm.tm_isdst;
		newtm.tm_year = nowtm.tm_year;
		newtm.tm_mon = nowtm.tm_mon;
		newtm.tm_mday = nowtm.tm_mday;
		newtm.tm_hour = ltime.tm_hour;
		newtm.tm_min = ltime.tm_min;
		newtm.tm_sec = ltime.tm_sec;

		newtmt = mktime( & newtm );

		if(newtmt <= now)
			newtmt += 86400 ;
		return newtmt;
	}
	else if( ck->type == CLOCK_TYPE_LOGIN || ck->type == CLOCK_TYPE_LOGIN_HAD ){
		return (ck->clock_time + currentuser->lastlogin) ;
	}
	else if( ck->type == CLOCK_TYPE_WEEK ){
		struct tm newtm;
		struct tm ltime;
		struct tm nowtm;
		time_t now;
		time_t newtmt;

		now = time(0);

		localtime_r( & (ck->clock_time) ,&ltime ) ;
		localtime_r( & now , &nowtm );

		newtm.tm_isdst = nowtm.tm_isdst;
		newtm.tm_year = nowtm.tm_year;
		newtm.tm_mon = nowtm.tm_mon;
		newtm.tm_mday = nowtm.tm_mday;
		newtm.tm_hour = ltime.tm_hour;
		newtm.tm_min = ltime.tm_min;
		newtm.tm_sec = ltime.tm_sec;

		newtmt = mktime( & newtm );
		newtmt += 86400 * (ltime.tm_wday - nowtm.tm_wday) ;

		if( newtmt < now)
			newtmt += 86400 * 7 ;

		return newtmt;
	}
	else if( ck->type == CLOCK_TYPE_MONTH ){
		struct tm newtm;
		struct tm ltime;
		struct tm nowtm;
		time_t now;
		time_t newtmt;

		now = time(0);
		localtime_r( & (ck->clock_time) ,&ltime ) ;
		localtime_r( & now , &nowtm );

		newtm.tm_isdst = nowtm.tm_isdst;
		newtm.tm_year = nowtm.tm_year;
		newtm.tm_mon = nowtm.tm_mon;
		newtm.tm_mday = ltime.tm_mday;
		newtm.tm_hour = ltime.tm_hour;
		newtm.tm_min = ltime.tm_min;
		newtm.tm_sec = ltime.tm_sec;

		while(1){

			newtmt = mktime( & newtm );

			if( newtmt < now || localtime(&newtmt)->tm_mday != ltime.tm_mday ){
				if(newtm.tm_mon < 11) newtm.tm_mon++;
				else{
					newtm.tm_mon = 0;
					newtm.tm_year++;
				}
				continue;
			}

			break;
		}

		return newtmt;
	}
	return ck->clock_time;
}

/*
 * mode:  1: µÇÂ½µÚÒ»´ÎÔËĞĞ
 *        2: Ôö¼Ó/¼õÉÙ ÄÖÁåÊ±ÖØĞÂ¼ÆËã,²»ÓÃold_calltime
 *        0: ÄÖÁå¹ıºóÖØĞÂ¼ÆËã
 */
time_t calc_calltime(int mode)
{
	int out=0;
	int i;
	time_t ret=0;
	time_t old_calltime ;

	if(mode==0) old_calltime = calltime;
	else old_calltime = 0;

	if(clock_data == NULL){
		clock_data = (struct clock_struct *) malloc(sizeof(struct clock_struct) * MAX_CLOCK_DEF);
		if(clock_data ==NULL)
			return 0;
		out = 1;
	}

	init_clock_data();

	for(i=0;i<clock_total;i++){
		time_t realcalltime;

		realcalltime = get_realcalltime(clock_data + i);

		if(mode==1 && (clock_data+i)->type == CLOCK_TYPE_LOGIN_HAD) 
			(clock_data+i)->type = CLOCK_TYPE_LOGIN;

		if((clock_data+i)->type == CLOCK_TYPE_LOGIN){
			if(mode == 0 && old_calltime == realcalltime){
				(clock_data+i)->type = CLOCK_TYPE_LOGIN_HAD;
				continue;
			}
			if((ret == 0 || ret > realcalltime) && realcalltime > time(0) ){
				ret = realcalltime;
				strncpy(calltimememo,(clock_data+i)->memo,40);
			}
			continue;
		}

		if(realcalltime < time(0)){
			if((clock_data+i)->type == CLOCK_TYPE_IMP_NOT ){
				if(mode == 0 && old_calltime == realcalltime){
					(clock_data+i)->type = CLOCK_TYPE_IMP_HAD;
					continue;
				}
				if(ret==0 || ret > realcalltime){
					ret = realcalltime;
					strncpy(calltimememo,(clock_data+i)->memo,40);
				}
			}
			continue;
		}
		if(ret == 0 || realcalltime < ret ){
			ret = realcalltime;
			strncpy(calltimememo,(clock_data+i)->memo,40);
		}
	}

	save_clock_data();
	if(out){
		free(clock_data);
		clock_data=NULL;
	}

	calltime = ret;

	return ret;
}

static int add_new_clock()
{

	char ans[3];
	int setok=0;
	char memo[40];
	time_t now;
	struct clock_struct ck;

	if( clock_total >= MAX_CLOCK_DEF )
		return SHOW_CONTINUE;

	clear();
	move(1,0);
	prints("1. ÆÕÍ¨ÄÖÁå (ÉèÖÃÏµÍ³¼¸·ÖÖÓºóÌáĞÑÄú,²»ÔÚÏßºöÂÔ)\n");
	prints("2. ¶¨Ê±ÄÖÁå (Ä³ÄêÄ³ÔÂÄ³ÈÕÄ³·ÖÌáĞÑÄú,²»ÔÚÏßºöÂÔ)\n");
	prints("3. Ã¿ÈÕÄÖÁå\n");
	prints("4. Ã¿ÖÜÄÖÁå\n");
	prints("5. Ã¿ÔÂÄÖÁå\n");
	prints("6. ÖØÒªÄÖÁå (ÄÖÁåÊ±Î´ÔÚÏßÏÂ´ÎµÇÂ½Ê±×Ô¶¯ÌáÊ¾)\n");
	prints("7. Í£ÁôÄÖÁå (ÔÚBBSÉÏÍ£Áôµ½´ï¼¸·ÖÖÓÊ±ÌáĞÑ)");
	getdata(10,0,"ÇëÑ¡ÔñÄÖÁåÖÖÀà(1-7)? [0]:",ans,sizeof(ans),DOECHO,NULL,true);

	switch( ans[0] ){
	case '1':
	{
		char tmp[6];
		int tt;
		getdata(11,0,"¼¸·ÖÖÓºóÏµÍ³ÄÖÁå:",tmp,4,DOECHO,NULL,true);
		tt = atoi(tmp);
		if(tt <= 0)
			break;
		getdata(12,0,"ÏµÍ³ÄÖÁå×Ô¶¨ÒåÌáĞÑÄÚÈİ:",memo,40,DOECHO,NULL,true);
		if(memo[0]=='\0' || memo[0]=='\r' || memo[0]=='\n')
			strcpy(memo,"ÏµÍ³ÄÖÁåà¶~~~~~~~~~~~~");
		now=time(0);
		ck.type = CLOCK_TYPE_NORMAL;
		ck.clock_time = now + tt * 60 ;
		strncpy(ck.memo,memo,40);
		ck.memo[39]='\0';
		setok = 1;
		break;
	}
	case '7':
	{
		char tmp[6];
		int tt;
		getdata(11,0,"Ã¿´ÎÉÏÕ¾¼¸·ÖÖÓºóÏµÍ³ÄÖÁå:",tmp,4,DOECHO,NULL,true);
		tt = atoi(tmp);
		if(tt <= 0)
			break;
		getdata(12,0,"ÏµÍ³ÄÖÁå×Ô¶¨ÒåÌáĞÑÄÚÈİ:",memo,40,DOECHO,NULL,true);
		if(memo[0]=='\0' || memo[0]=='\r' || memo[0]=='\n')
			strcpy(memo,"ÏµÍ³ÄÖÁåà¶~~~~~~~~~~~~");
		ck.type = CLOCK_TYPE_LOGIN;
		ck.clock_time = tt * 60 ;
		strncpy(ck.memo,memo,40);
		ck.memo[39]='\0';
		setok = 1;
		break;
	}
	case '2':
	case '6':
	{
		struct tm *ltime;
		struct tm newtm;
		char buf[80];
		char tmp[6];
		int tt;
		now = time(0);
		ltime = localtime( &now );
		newtm.tm_isdst = ltime->tm_isdst;
		
		sprintf(buf,"Éè¶¨ÏµÍ³ÄÖÁåÄê·İ [%d]:",ltime->tm_year+1900);
		getdata(11,0,buf,tmp,5,DOECHO,NULL,true);
		if(tmp[0]=='\0' || tmp[0]=='\r' || tmp[0]=='\n')
			newtm.tm_year = ltime->tm_year;
		else{
			tt = atoi(tmp);
			if( tt <= 1900 || tt >= 2010 )
				break;
			newtm.tm_year = tt-1900;
		}

		sprintf(buf,"Éè¶¨ÏµÍ³ÄÖÁåÔÂ·İ [%d]:",ltime->tm_mon+1);
		getdata(12,0,buf,tmp,3,DOECHO,NULL,true);
		if(tmp[0]=='\0' || tmp[0]=='\r' || tmp[0]=='\n')
			newtm.tm_mon = ltime->tm_mon;
		else{
			tt = atoi(tmp);
			if(tt <= 0 || tt > 12)
				break;
			newtm.tm_mon = tt-1;
		}
		
		sprintf(buf,"Éè¶¨ÏµÍ³ÄÖÁåÈÕ×Ó [%d]:",ltime->tm_mday);
		getdata(13,0,buf,tmp,3,DOECHO,NULL,true);
		if(tmp[0]=='\0' || tmp[0]=='\r' || tmp[0]=='\n')
			newtm.tm_mday = ltime->tm_mday;
		else{
			tt = atoi(tmp);
			if(tt <= 0 || tt > 31)
				break;
			newtm.tm_mday = tt;
		}

		sprintf(buf,"Éè¶¨ÏµÍ³ÄÖÁåĞ¡Ê± [%d]:",ltime->tm_hour);
		getdata(14,0,buf,tmp,3,DOECHO,NULL,true);
		if(tmp[0]=='\0' || tmp[0]=='\r' || tmp[0]=='\n')
			newtm.tm_hour = ltime->tm_hour;
		else{
			tt = atoi(tmp);
			if(tt < 0 || tt > 23)
				break;
			newtm.tm_hour = tt;
		}

		sprintf(buf,"Éè¶¨ÏµÍ³ÄÖÁå·ÖÖÓ [%d]:",ltime->tm_min);
		getdata(15,0,buf,tmp,3,DOECHO,NULL,true);
		if(tmp[0]=='\0' || tmp[0]=='\r' || tmp[0]=='\n')
			newtm.tm_min = ltime->tm_min;
		else{
			tt = atoi(tmp);
			if(tt < 0 || tt > 59)
				break;
			newtm.tm_min = tt;
		}

		newtm.tm_sec = 0;

		getdata(16,0,"ÏµÍ³ÄÖÁå×Ô¶¨ÒåÌáĞÑÄÚÈİ:",memo,40,DOECHO,NULL,true);
		if(memo[0]=='\0' || memo[0]=='\r' || memo[0]=='\n')
			strcpy(memo,"ÏµÍ³ÄÖÁåà¶~~~~~~~~~~~~");

		ck.clock_time = mktime(&newtm);
		if(ck.clock_time <= time(0))
			break;

		if(ans[0]=='2') ck.type = CLOCK_TYPE_NORMAL ;
		else ck.type = CLOCK_TYPE_IMP_NOT ;
		strncpy(ck.memo,memo,40);
		ck.memo[39]='\0';
		setok = 1;
		break;
	}
	case '3':
	{
		struct tm *ltime;
		struct tm newtm;
		char buf[80];
		char tmp[6];
		int tt;

		now = time(0);
		ltime = localtime( &now );
		newtm.tm_isdst = ltime->tm_isdst;
		newtm.tm_year = ltime->tm_year;
		newtm.tm_mon = ltime->tm_mon;
		newtm.tm_mday = ltime->tm_mday;

		sprintf(buf,"Éè¶¨ÏµÍ³ÄÖÁåĞ¡Ê± [%d]:",ltime->tm_hour);
		getdata(11,0,buf,tmp,3,DOECHO,NULL,true);
		if(tmp[0]=='\0' || tmp[0]=='\r' || tmp[0]=='\n')
			newtm.tm_hour = ltime->tm_hour;
		else{
			tt = atoi(tmp);
			if(tt < 0 || tt > 23)
				break;
			newtm.tm_hour = tt;
		}

		sprintf(buf,"Éè¶¨ÏµÍ³ÄÖÁå·ÖÖÓ [%d]:",ltime->tm_min);
		getdata(12,0,buf,tmp,3,DOECHO,NULL,true);
		if(tmp[0]=='\0' || tmp[0]=='\r' || tmp[0]=='\n')
			newtm.tm_min = ltime->tm_min;
		else{
			tt = atoi(tmp);
			if(tt < 0 || tt > 59)
				break;
			newtm.tm_min = tt;
		}

		newtm.tm_sec = 0;

		getdata(13,0,"ÏµÍ³ÄÖÁå×Ô¶¨ÒåÌáĞÑÄÚÈİ:",memo,40,DOECHO,NULL,true);
		if(memo[0]=='\0' || memo[0]=='\r' || memo[0]=='\n')
			strcpy(memo,"ÏµÍ³ÄÖÁåà¶~~~~~~~~~~~~");

		ck.clock_time = mktime(&newtm);
		if(ck.clock_time <= 0)
			break;
		ck.type = CLOCK_TYPE_DAY;
		strncpy(ck.memo,memo,40);
		ck.memo[39]='\0';
		setok = 1;
		break;
	}
	case '4':
	{
		struct tm *ltime;
		struct tm newtm;
		char buf[80];
		char tmp[6];
		int tt;
		int wday;

		now = time(0);
		ltime = localtime( &now );
		newtm.tm_isdst = ltime->tm_isdst;
		newtm.tm_year = ltime->tm_year;
		newtm.tm_mon = ltime->tm_mon;
		newtm.tm_mday = ltime->tm_mday;

		sprintf(buf,"Éè¶¨ÏµÍ³ÄÖÁåÔÚÃ¿ÖÜĞÇÆÚ¼¸ [%d]:",ltime->tm_wday?ltime->tm_wday:7);
		getdata(11,0,buf,tmp,3,DOECHO,NULL,true);
		if(tmp[0]=='\0' || tmp[0]=='\r' || tmp[0]=='\n'){
			newtm.tm_wday = ltime->tm_wday;
			wday = ltime->tm_wday;
		}
		else{
			tt = atoi(tmp);
			if(tt < 0 || tt > 7)
				break;
			newtm.tm_wday = tt%7;
			wday = tt%7;
		}

		sprintf(buf,"Éè¶¨ÏµÍ³ÄÖÁåĞ¡Ê± [%d]:",ltime->tm_hour);
		getdata(12,0,buf,tmp,3,DOECHO,NULL,true);
		if(tmp[0]=='\0' || tmp[0]=='\r' || tmp[0]=='\n')
			newtm.tm_hour = ltime->tm_hour;
		else{
			tt = atoi(tmp);
			if(tt < 0 || tt > 23)
				break;
			newtm.tm_hour = tt;
		}

		sprintf(buf,"Éè¶¨ÏµÍ³ÄÖÁå·ÖÖÓ [%d]:",ltime->tm_min);
		getdata(13,0,buf,tmp,3,DOECHO,NULL,true);
		if(tmp[0]=='\0' || tmp[0]=='\r' || tmp[0]=='\n')
			newtm.tm_min = ltime->tm_min;
		else{
			tt = atoi(tmp);
			if(tt < 0 || tt > 59)
				break;
			newtm.tm_min = tt;
		}

		newtm.tm_sec = 0;

		getdata(14,0,"ÏµÍ³ÄÖÁå×Ô¶¨ÒåÌáĞÑÄÚÈİ:",memo,40,DOECHO,NULL,true);
		if(memo[0]=='\0' || memo[0]=='\r' || memo[0]=='\n')
			strcpy(memo,"ÏµÍ³ÄÖÁåà¶~~~~~~~~~~~~");

		ck.clock_time = mktime(&newtm);
		ck.clock_time += ( wday - ltime->tm_wday ) * 86400;
		if(ck.clock_time <= 0)
			break;
		ck.type = CLOCK_TYPE_WEEK;
		strncpy(ck.memo,memo,40);
		ck.memo[39]='\0';
		setok = 1;
		break;
	}
	case '5':
	{
		struct tm *ltime;
		struct tm newtm;
		char buf[80];
		char tmp[6];
		int tt;

		now = time(0);
		ltime = localtime( &now );
		newtm.tm_isdst = ltime->tm_isdst;
		newtm.tm_year = ltime->tm_year;
		newtm.tm_mon = ltime->tm_mon;
		newtm.tm_mday = ltime->tm_mday;

		sprintf(buf,"Éè¶¨ÏµÍ³ÄÖÁåÔÚÃ¿ÔÂ¼¸ºÅ [%d]:",ltime->tm_mday);
		getdata(11,0,buf,tmp,3,DOECHO,NULL,true);
		if(tmp[0]=='\0' || tmp[0]=='\r' || tmp[0]=='\n'){
			newtm.tm_mday = ltime->tm_mday;
		}
		else{
			tt = atoi(tmp);
			if(tt <= 0 || tt > 31)
				break;
			newtm.tm_mday = tt;
		}

		sprintf(buf,"Éè¶¨ÏµÍ³ÄÖÁåĞ¡Ê± [%d]:",ltime->tm_hour);
		getdata(12,0,buf,tmp,3,DOECHO,NULL,true);
		if(tmp[0]=='\0' || tmp[0]=='\r' || tmp[0]=='\n')
			newtm.tm_hour = ltime->tm_hour;
		else{
			tt = atoi(tmp);
			if(tt < 0 || tt > 23)
				break;
			newtm.tm_hour = tt;
		}

		sprintf(buf,"Éè¶¨ÏµÍ³ÄÖÁå·ÖÖÓ [%d]:",ltime->tm_min);
		getdata(13,0,buf,tmp,3,DOECHO,NULL,true);
		if(tmp[0]=='\0' || tmp[0]=='\r' || tmp[0]=='\n')
			newtm.tm_min = ltime->tm_min;
		else{
			tt = atoi(tmp);
			if(tt < 0 || tt > 59)
				break;
			newtm.tm_min = tt;
		}

		newtm.tm_sec = 0;

		getdata(14,0,"ÏµÍ³ÄÖÁå×Ô¶¨ÒåÌáĞÑÄÚÈİ:",memo,40,DOECHO,NULL,true);
		if(memo[0]=='\0' || memo[0]=='\r' || memo[0]=='\n')
			strcpy(memo,"ÏµÍ³ÄÖÁåà¶~~~~~~~~~~~~");

		ck.clock_time = mktime(&newtm);
		if(ck.clock_time <= 0)
			break;
		ck.type = CLOCK_TYPE_MONTH;
		strncpy(ck.memo,memo,40);
		ck.memo[39]='\0';
		setok = 1;
		break;
	}
	default:
		break;
	}

	if(!setok) return SHOW_DIRCHANGE;

	add_clock_data(&ck);

	calc_calltime(2);

	return SHOW_DIRCHANGE;
}

static void get_clock_string(struct clock_struct * ck, char *typestr, char * timestr)
{
	char *c;

	switch(ck->type){
	case CLOCK_TYPE_DAY:
	{
		struct tm tmm;
		localtime_r(&(ck->clock_time),&tmm);
		strcpy(typestr,"Ã¿ÈÕÄÖÖÓ");
		sprintf(timestr,"Ã¿ÈÕ%2dÊ±%2d·Ö",tmm.tm_hour,tmm.tm_min);
		break;
	}
	case CLOCK_TYPE_WEEK:
	{
		struct tm tmm;
		localtime_r(&(ck->clock_time),&tmm);
		strcpy(typestr,"Ã¿ÖÜÄÖÖÓ");
		sprintf(timestr,"Ã¿ÖÜ%d %2dÊ±%2d·Ö",tmm.tm_wday,tmm.tm_hour,tmm.tm_min);
		break;
	}
	case CLOCK_TYPE_MONTH:
	{
		struct tm tmm;
		localtime_r(&(ck->clock_time),&tmm);
		strcpy(typestr,"Ã¿ÔÂÄÖÖÓ");
		sprintf(timestr,"Ã¿ÔÂ%2dÈÕ%2dÊ±%2d·Ö",tmm.tm_mday,tmm.tm_hour,tmm.tm_min);
		break;
	}
	case CLOCK_TYPE_IMP_NOT:
		strcpy(typestr,"ÖØÒªÄÖÖÓ");
		strcpy(timestr,ctime(&(ck->clock_time)));
		break;
	case CLOCK_TYPE_IMP_HAD:
		strcpy(typestr,"ÖØÒªÄÖÁå");
		strcpy(timestr,ctime(&(ck->clock_time)));
		break;
	case CLOCK_TYPE_LOGIN:
		strcpy(typestr,"Í£ÁôÄÖÖÓ");
		sprintf(timestr,"ÉÏÕ¾Í£Áô%d·ÖÖÓºó",ck->clock_time/60);
		break;
	case CLOCK_TYPE_LOGIN_HAD:
		strcpy(typestr,"Í£ÁôÄÖÁå");
		sprintf(timestr,"ÉÏÕ¾Í£Áô%d·ÖÖÓºó",ck->clock_time/60);
		break;
	default:
		strcpy(typestr,"ÆÕÍ¨ÄÖÖÓ");
		strcpy(timestr,ctime(&(ck->clock_time)));
		break;
	}

	if((c=strchr(timestr,'\r'))!=NULL) *c='\0';
	if((c=strchr(timestr,'\n'))!=NULL) *c='\0';
}

static int set_clock_select(struct _select_def *conf)
{
	char clocktypestring[10];
	char clocktimestring[50];

	get_clock_string(clock_data+conf->pos-1,clocktypestring,clocktimestring);

	clear();
	move(1,0);

	prints("%s\n\n",clocktypestring);
	prints("%s\n\n",clocktimestring);
	prints("memo:%s\n",(clock_data+conf->pos-1)->memo);
		
	pressanykey();

	return SHOW_REFRESH;
}

static int set_clock_show(struct _select_def *conf, int i)
{
	char typestr[10];
	char timestr[50];

	get_clock_string(clock_data+i-1,typestr,timestr);
	typestr[4]=='\0';
	prints(" %-4s %-25s %-40s",typestr,timestr,(clock_data+i-1)->memo);
	return SHOW_CONTINUE;
}

static int set_clock_prekey(struct _select_def *conf, int *key)
{
	switch (*key) {
	case 'q':
		*key = KEY_LEFT;
		break;
	case 'p':
	case 'k':
		*key = KEY_UP;
		break;
	case ' ':
	case 'N':
		*key = KEY_PGDN;
		break;
	case 'n':
	case 'j':
		*key = KEY_DOWN;
		break;
	}
	return SHOW_CONTINUE;
}

static int set_clock_refresh(struct _select_def *conf)
{
	clear();
	docmdtitle("[ÏµÍ³ÄÖÖÓÉèÖÃ]","[a Ôö¼Ó] [d É¾³ı] [s ²éÑ¯×î½üÒ»¸öÄÖÁåÊ±¼ä]");
	move(2,0);
	prints("[0;1;37;44m   %-4s %-25s %-40s[m","ÀàĞÍ¡","ÄÖÁåÊ±¼ä","ÄÖÁåËµÃ÷");
	update_endline();
	return SHOW_CONTINUE;
}

static int set_clock_getdata(struct _select_def *conf,int pos,int len)
{
	conf->item_count = clock_total;

	return SHOW_CONTINUE;
}

static int set_clock_key(struct _select_def *conf, int key)
{
	switch (key){

	case 'a':
		return add_new_clock();
		break;
	case 'd':
		return del_clock(conf->pos-1);
		break;
	case 's':
		clear();
		move(1,0);
		if(calltime)
			prints("ÏÂÒ»¸öÄÖÖÓÊ±¿Ì:%s",ctime(&calltime));
		else
			prints("ÏÖÔÚÏµÍ³Ã»ÓĞÄÖÖÓ");
		pressanykey();
		return SHOW_DIRCHANGE;
	}

	return SHOW_CONTINUE;
}

int set_clock()
{

	struct _select_def group_conf;
	int i;
	POINT *pts;

	clock_data = (struct clock_struct *) malloc(sizeof(struct clock_struct) * MAX_CLOCK_DEF);
	if(clock_data==NULL) return -1;

	if(init_clock_data()==0) return -2;

	bzero(&group_conf,sizeof(struct _select_def));
	group_conf.item_count = clock_total;

    //TODO: ´°¿Ú´óĞ¡¶¯Ì¬¸Ä±äµÄÇé¿ö£¿ÕâÀïÓĞbug
    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 2;
        pts[i].y = i + 3;
    }
	group_conf.item_per_page = BBS_PAGESIZE;
    group_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    group_conf.prompt = "¡ô";
    group_conf.item_pos = pts;
	group_conf.title_pos.x = 0;
	group_conf.title_pos.y = 0;
	group_conf.pos=1;
	group_conf.page_pos=1;

	group_conf.on_select = set_clock_select;
	group_conf.show_data = set_clock_show;
	group_conf.pre_key_command = set_clock_prekey;
	group_conf.show_title = set_clock_refresh;
	group_conf.get_data = set_clock_getdata;
	group_conf.key_command = set_clock_key;

	list_select_loop(&group_conf);
	save_clock_data();
	free(pts);
	free(clock_data);
	clock_data=NULL;

	return 1;
}
