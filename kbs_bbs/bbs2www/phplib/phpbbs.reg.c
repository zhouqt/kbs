#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_smth_bbs.h"  

#include "bbs.h"
#include "bbslib.h"



PHP_FUNCTION(bbs_saveuserdata)
{
    char*   userid,
	    *   realname,
        *   photo_url,
        *   address,
		*	email,
		*	phone,
		*   mobile_phone,
		* OICQ, 
		* ICQ, 
		* MSN, 
		* homepage,
		* userface_url,
		* country,
		* province,
		* city,
		* graduate_school;
    int     userid_len,
	        realname_len,
	        photo_url_len,
			address_len,
			email_len,
			phone_len,
			mobile_phone_len,
			OICQ_len,
			ICQ_len,
			MSN_len,
			homepage_len,
			userface_url_len,
			country_len,
			province_len,
			city_len,
			graduate_school_len;
	long	gender,
	        year,
	        month,
			day,
			userface_img,
			userface_width,
			userface_height, 
			group,
			shengxiao,
			bloodtype,
			religion ,
			profession, 
			married, 
			education,
			character;
    zend_bool   bAuto;
	struct  userdata ud;
	int     usernum;

    int ac = ZEND_NUM_ARGS();


	if (ac != 32 || zend_parse_parameters(32 TSRMLS_CC, "sssllllssssssslslllsssllllllslsb", &userid,&userid_len,&realname,&realname_len,
	    &address,&address_len,&gender,&year,&month,&day,&email,&email_len,&phone,&phone_len,&mobile_phone,&mobile_phone_len,
		&OICQ, &OICQ_len, &ICQ, &ICQ_len, &MSN, &MSN_len, &homepage, &homepage_len, &userface_img,
		&userface_url, &userface_url_len, &userface_width, &userface_height, &group, &country, &country_len,
		&province, &province_len, &city, &city_len, &shengxiao, &bloodtype, &religion, &profession,
		&married, &education, &graduate_school, &graduate_school_len, &character, &photo_url, &photo_url_len,&bAuto) == FAILURE)
    {
		WRONG_PARAM_COUNT;
	}

	if(userid_len > IDLEN)RETURN_LONG(2);

    usernum = searchuser(userid);
	if(0 == usernum)RETURN_LONG(3);

	if (userface_url_len!=0) {
		userface_img=-1;
		if ( (userface_width<0) || (userface_width>120) ){
			RETURN_LONG(-1);
		}
		if ( (userface_height<0) || (userface_height>120) ){
			RETURN_LONG(-2);
		}
	} else {
		userface_width=0;
		userface_height=0;
	}

	if( read_user_memo(userid, &(getSession()->currentmemo)) <= 0) RETURN_LONG(-2);
	read_userdata(userid, &ud);
    strncpy(ud.realname, realname, NAMELEN);
    strncpy(ud.address, address, STRLEN);
	strncpy(ud.reg_email,email,STRLEN);
	strncpy(ud.OICQ,OICQ,STRLEN);
	strncpy(ud.ICQ,ICQ,STRLEN);
	strncpy(ud.MSN,MSN,STRLEN);
	strncpy(ud.homepage,homepage,STRLEN);
	strncpy(ud.userface_url,userface_url,STRLEN);
	strncpy(ud.country,country,STRLEN);
	strncpy(ud.province,province,STRLEN);
	strncpy(ud.city,city,STRLEN);
	strncpy(ud.graduateschool,graduate_school,STRLEN);
	strncpy(ud.telephone,phone,STRLEN);
	strncpy(ud.photo_url,photo_url,STRLEN);
	ud.photo_url[STRLEN-1]=0;
	ud.telephone[STRLEN-1]=0;
	ud.OICQ[STRLEN-1]=0;
	ud.ICQ[STRLEN-1]=0;
	ud.MSN[STRLEN-1]=0;
	ud.homepage[STRLEN-1]=0;
	ud.userface_url[STRLEN-1]=0;
	ud.country[STRLEN-1]=0;
	ud.province[STRLEN-1]=0;
	ud.city[STRLEN-1]=0;
	ud.graduateschool[STRLEN-1]=0;
    ud.realname[NAMELEN-1] = '\0';
	ud.address[STRLEN-1] = '\0';
	ud.reg_email[STRLEN-1] = '\0';

    
#ifdef HAVE_BIRTHDAY
    ud.birthyear=(year > 1900 && year < 2050)?(year-1900):0;
	ud.birthmonth=(month >=1 && month <=12)?month:0;
	ud.birthday=(day>=1 && day <=31)?day:0;
	if(gender==1)ud.gender='M';
	else
	    ud.gender='F';
#endif
	ud.userface_img=userface_img;
	ud.userface_width=userface_width;
	ud.userface_height=userface_height;
	ud.group=group;
	ud.shengxiao=shengxiao;
	ud.bloodtype=bloodtype;
	ud.religion=religion;
	ud.profession=profession;
	ud.married=married;
	ud.education=education;
	ud.character=character;
	memcpy(&((getSession()->currentmemo)->ud), &ud, sizeof(ud));
	end_mmapfile((getSession()->currentmemo), sizeof(struct usermemo), -1);
	write_userdata(userid, &ud);
    RETURN_LONG(0);

}

/**
 * Function: Create a registry form
 *  prototype:
 *
 * #ifdef HAVE_WFORUM
 *
 * int bbs_createregform(string userid ,string realname,string dept,string address,int gender,int year,int month,int day,
    string email,string phone,string mobile_phone,string OICQ, string ICQ, string MSN, string homepage, int userface_img,
	string userface_url,int userface_width, int userface_height, int group, string country ,string province, string city,
	int shengxiao, int bloodtype, int religion , int profession, int married, int education, string graduate_school,
	int character,	bool bAuto)
 *
 * 或者 (wForum 用下面的方式调用表示重新填写注册单)
 * #endif
 *
 * int bbs_createregform(string userid ,string realname,string dept,string address,int gender,int year,int month,int day,
    string email,string phone,string mobile_phone,bool bAuto)
 *
 *
 *  bAuto : true -- 自动生成注册单,false -- 手工.
 *  @return the result
 *  	0 -- success,
 *      1 -- 注册单尚未处理
 *      2 -- 参数错误
 *      3 -- 用户不存在
 *      4 -- 用户已经通过注册
 *      5 -- 不到时间
 *  	10 -- system error
 *  @author binxun 2003.5
 */
PHP_FUNCTION(bbs_createregform)
{
    char*   userid,
	    *   realname,
        *   dept,
        *   address,
		*	email,
		*	phone,
#ifdef HAVE_WFORUM
		* OICQ, 
		* ICQ, 
		* MSN, 
		* homepage,
		* userface_url,
		* country,
		* province,
		* city,
		* graduate_school,
#endif
		*   mobile_phone;
    int     userid_len,
	        realname_len,
	        dept_len,
			address_len,
			email_len,
			phone_len,
			mobile_phone_len;
#ifdef HAVE_WFORUM
    int     OICQ_len,
			ICQ_len,
			MSN_len,
			homepage_len,
			userface_url_len,
			country_len,
			province_len,
			city_len,
			graduate_school_len;
    long	userface_img,
			userface_width,
			userface_height, 
			group,
			shengxiao,
			bloodtype,
			religion ,
			profession, 
			married, 
			education,
			character;
#endif
	long	gender,
			year,
	        month,
			day;
    zend_bool   bAuto;
	struct  userdata ud;
	struct  userec* uc;
	FILE*   fn;
	char    genbuf[STRLEN+1];
	char*   ptr;
	int     usernum;
	long    now;
#ifdef HAVE_WFORUM
	int     bReFill;
#endif
    int ac = ZEND_NUM_ARGS();


#ifdef HAVE_WFORUM
	bReFill = (ac == 12);
	if (ac != 32 || zend_parse_parameters(32 TSRMLS_CC, "ssssllllssssssslslllsssllllllslb", &userid,&userid_len,&realname,&realname_len,&dept,&dept_len,
	    &address,&address_len,&gender,&year,&month,&day,&email,&email_len,&phone,&phone_len,&mobile_phone,&mobile_phone_len,
		&OICQ, &OICQ_len, &ICQ, &ICQ_len, &MSN, &MSN_len, &homepage, &homepage_len, &userface_img,
		&userface_url, &userface_url_len, &userface_width, &userface_height, &group, &country, &country_len,
		&province, &province_len, &city, &city_len, &shengxiao, &bloodtype, &religion, &profession,
		&married, &education, &graduate_school, &graduate_school_len, &character,&bAuto) == FAILURE)
#endif
    if (ac != 12 || zend_parse_parameters(12 TSRMLS_CC, "ssssllllsssb", &userid,&userid_len,&realname,&realname_len,&dept,&dept_len,
	    &address,&address_len,&gender,&year,&month,&day,&email,&email_len,&phone,&phone_len,&mobile_phone,&mobile_phone_len,&bAuto) == FAILURE)
    {
		WRONG_PARAM_COUNT;
	}

	if(userid_len > IDLEN)RETURN_LONG(2);

    usernum = searchuser(userid);
	if(0 == usernum)RETURN_LONG(3);

#ifdef HAVE_WFORUM
	if (!bReFill) {
		if (userface_url_len!=0) {
			userface_img=-1;
			if ( (userface_width<0) || (userface_width>120) ){
				RETURN_LONG(-1);
			}
			if ( (userface_height<0) || (userface_height>120) ){
				RETURN_LONG(-2);
			}
		} else {
			userface_width=0;
			userface_height=0;
		}
	}
#endif

        //检查用户是否已经通过注册或者还不到时间(先放到这里,最好放到php里面)
	    if(getuser(userid,&uc) == 0)RETURN_LONG(3);
		if(HAS_PERM(uc,PERM_LOGINOK))RETURN_LONG(4);

	if(!bAuto)
	{
		/* remed by roy 2003.7.17 
		if(time(NULL) - uc->firstlogin < REGISTER_WAIT_TIME)RETURN_LONG(5);
		*/
	    //检查是否单子已经填过了
		if ((fn = fopen("new_register", "r")) != NULL) {
			while (fgets(genbuf, STRLEN, fn) != NULL) {
				if ((ptr = strchr(genbuf, '\n')) != NULL)
					*ptr = '\0';
				if (strncmp(genbuf, "userid: ", 8) == 0 && strcmp(genbuf + 8, userid) == 0) {
					fclose(fn);
					RETURN_LONG(1);
				}
			}
			fclose(fn);
		}
    }
	if( read_user_memo(userid, &(getSession()->currentmemo)) <= 0) RETURN_LONG(-2);
	read_userdata(userid, &ud);
    strncpy(ud.realname, realname, NAMELEN);
    strncpy(ud.address, address, STRLEN);
	strncpy(ud.reg_email,email,STRLEN);
#ifdef HAVE_WFORUM
	if (!bReFill) {
		strncpy(ud.OICQ,OICQ,STRLEN);
		strncpy(ud.ICQ,ICQ,STRLEN);
		strncpy(ud.MSN,MSN,STRLEN);
		strncpy(ud.homepage,homepage,STRLEN);
		strncpy(ud.userface_url,userface_url,STRLEN);
		strncpy(ud.country,country,STRLEN);
		strncpy(ud.province,province,STRLEN);
		strncpy(ud.city,city,STRLEN);
		strncpy(ud.graduateschool,graduate_school,STRLEN);
		strncpy(ud.telephone,phone,STRLEN);
		ud.telephone[STRLEN-1]=0;
		ud.OICQ[STRLEN-1]=0;
		ud.ICQ[STRLEN-1]=0;
		ud.MSN[STRLEN-1]=0;
		ud.homepage[STRLEN-1]=0;
		ud.userface_url[STRLEN-1]=0;
		ud.country[STRLEN-1]=0;
		ud.province[STRLEN-1]=0;
		ud.city[STRLEN-1]=0;
		ud.graduateschool[STRLEN-1]=0;
		ud.userface_img=userface_img;
		ud.userface_width=userface_width;
		ud.userface_height=userface_height;
		ud.group=group;
		ud.shengxiao=shengxiao;
		ud.bloodtype=bloodtype;
		ud.religion=religion;
		ud.profession=profession;
		ud.married=married;
		ud.education=education;
		ud.character=character;
	}
#endif
    ud.realname[NAMELEN-1] = '\0';
	ud.address[STRLEN-1] = '\0';
	ud.reg_email[STRLEN-1] = '\0';

    if(strcmp(mobile_phone,"")){
	    ud.mobileregistered = true;
		strncpy(ud.mobilenumber,mobile_phone,MOBILE_NUMBER_LEN);
		ud.mobilenumber[MOBILE_NUMBER_LEN-1] = '\0';
	}
    else{
    	ud.mobileregistered = false;
    	}
    
#ifdef HAVE_BIRTHDAY
    ud.birthyear=(year > 1900 && year < 2050)?(year-1900):0;
	ud.birthmonth=(month >=1 && month <=12)?month:0;
	ud.birthday=(day>=1 && day <=31)?day:0;
	if(gender==1)ud.gender='M';
	else
	    ud.gender='F';
#endif
	memcpy(&((getSession()->currentmemo)->ud), &ud, sizeof(ud));
	end_mmapfile((getSession()->currentmemo), sizeof(struct usermemo), -1);
	write_userdata(userid, &ud);

#ifdef NEW_COMERS
	{
	FILE *fout;
	char buf2[STRLEN],buf[STRLEN];
	sprintf(buf, "tmp/newcomer.%s",uc->userid);
		if ((fout = fopen(buf, "w")) != NULL)
		{
			fprintf(fout, "大家好,\n\n");
			fprintf(fout, "我是 %s (%s), 来自 %s\n", uc->userid,
					uc->username, SHOW_USERIP(getCurrentUser(), getSession()->fromhost));
			fprintf(fout, "今天%s初来此站报到, 请大家多多指教。\n",
#ifdef HAVE_BIRTHDAY
					(ud.gender == 'M') ? "小弟" : "小女子");
#else
                                        "小弟");
#endif
			fprintf(fout, "\n\n我是www注册用户~~~\n\n");
			fclose(fout);
			sprintf(buf2, "新手上路: %s", uc->username);
			post_file(uc, "", buf, "newcomers", buf2, 0, 2, getSession());
			unlink(buf);
		}
	}
#endif

	sprintf(genbuf,"%ld.%ld.%ld",year,month,day);
	if(bAuto)
        fn = fopen("pre_register", "a");
	else
	    fn = fopen("new_register", "a");

    if (fn) {
        now = time(NULL);
        flock(fileno(fn),LOCK_EX);
        fprintf(fn, "usernum: %d, %s", usernum, ctime(&now));
        fprintf(fn, "userid: %s\n", userid);
        fprintf(fn, "realname: %s\n", realname);
        fprintf(fn, "career: %s\n", dept);
        fprintf(fn, "addr: %s\n", address);
        fprintf(fn, "phone: %s\n", phone);
        fprintf(fn, "birth: %s\n", genbuf);
        fprintf(fn, "----\n");
        flock(fileno(fn),LOCK_UN);
        fclose(fn);
        RETURN_LONG(0);
    }
	else
        RETURN_LONG(10);
}
