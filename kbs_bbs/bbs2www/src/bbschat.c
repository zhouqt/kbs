/*
 * $Id$
 */
#include "bbslib.h"
#include "netinet/in.h"
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#ifdef FREEBSD
#include <sys/socket.h>		/* need for FreeBSD */
#endif
#ifdef AIX
#include <sys/socket.h>
#include <sys/socketvar.h>
#endif

int chat_fd;
int test = 0;
char genbuf[1024];

struct action {
    char *verb;			/* ¶¯´Ê */
    char *part1_msg;		/* ½é´Ê */
    char *part2_msg;		/* ¶¯×÷ */
};


struct action party_data[] = {
    {"?", "ºÜÒÉ»óµÄ¿´×Å", ""},
    {"admire", "¶Ô", "µÄ¾°ÑöÖ®ÇéÓÌÈçÌÏÌÏ½­Ë®Á¬Ãà²»¾ø"},
    {"agree", "ÍêÈ«Í¬Òâ", "µÄ¿´·¨"},
    {"bearhug", "ÈÈÇéµÄÓµ±§", ""},
    {"bless", "×£¸£", "ĞÄÏëÊÂ³É"},
    {"bow", "±Ï¹ª±Ï¾´µÄÏò", "¾Ï¹ª"},
    {"bye", "¿´×Å", "µÄ±³Ó°£¬ÆàÈ»ÀáÏÂ¡£ÉíºóµÄÊÕÒô»ú´«À´µËÀö¾ıµÄ¸èÉù:\\n\"[31mºÎÈÕ¾ıÔÙÀ´.....[m\""},
    {"caress", "ÇáÇáµÄ¸§Ãş", ""},
    {"cat", "ÏñÖ»Ğ¡Ã¨°ãµØÒÀÙËÔÚ", "µÄ»³ÀïÈö½¿¡£"},
    {"cringe", "Ïò", "±°¹ªÇüÏ¥£¬Ò¡Î²ÆòÁ¯"},
    {"cry", "Ïò", "º¿ßû´ó¿Ş"},
    {"comfort", "ÎÂÑÔ°²Î¿", ""},
    {"clap", "Ïò", "ÈÈÁÒ¹ÄÕÆ"},
    {"dance", "À­×Å", "µÄÊÖôæôæÆğÎè"},
    {"dogleg", "¶Ô", "¹·ÍÈ"},
    {"drivel", "¶ÔÖø", "Á÷¿ÚË®"},
    {"dunno", "µÉ´óÑÛ¾¦£¬ÌìÕæµØÎÊ£º", "£¬ÄãËµÊ²÷áÎÒ²»¶®Ò®... :("},
    {"faint", "ÔÎµ¹ÔÚ", "µÄ»³Àï"},
    {"fear", "¶Ô", "Â¶³öÅÂÅÂµÄ±íÇé"},
    {"fool", "Çë´ó¼Ò×¢Òâ", "Õâ¸ö´ó°×³Õ....\\nÕæÊÇÌìÉÏÉÙÓĞ....ÈË¼ä½ö´æµÄ»î±¦....\\n²»¿´Ì«¿ÉÏ§ÁË£¡"},
    {"forgive", "´ó¶ÈµÄ¶Ô", "Ëµ£ºËãÁË£¬Ô­ÁÂÄãÁË"},
    {"giggle", "¶ÔÖø", "ÉµÉµµÄ´ôĞ¦"},
    {"grin", "¶Ô", "Â¶³öĞ°¶ñµÄĞ¦Èİ"},
    {"growl", "¶Ô", "ÅØÏø²»ÒÑ"},
    {"hand", "¸ú", "ÎÕÊÖ"},
    {"hammer", "¾ÙÆğºÃ´óºÃ´óµÄÌú´¸£¡£¡ÍÛ£¡Íù",
     "Í·ÉÏÓÃÁ¦Ò»ÇÃ£¡\\n***************\\n*  5000000 Pt *\\n***************\\n      | |      %1¡ï%2¡î%3¡ï%4¡î%5¡ï%6¡î%0\\n      | |         ºÃ¶àµÄĞÇĞÇÓ´\\n      |_|"},
    {"heng", "¿´¶¼²»¿´", "Ò»ÑÛ£¬ ºßÁËÒ»Éù£¬¸ß¸ßµÄ°ÑÍ·ÑïÆğÀ´ÁË,²»Ğ¼Ò»¹ËµÄÑù×Ó..."},
    {"hug", "ÇáÇáµØÓµ±§", ""},
    {"idiot", "ÎŞÇéµØ³ÜĞ¦", "µÄ³Õ´ô¡£"},
    {"kick", "°Ñ", "ÌßµÄËÀÈ¥»îÀ´"},
    {"kiss", "ÇáÎÇ", "µÄÁ³¼Õ"},
    {"laugh", "´óÉù³°Ğ¦", ""},
    {"lovelook", "À­×Å", "µÄÊÖ£¬ÎÂÈáµØÄ¬Ä¬¶ÔÊÓ¡£Ä¿¹âÖĞÔĞº¬×ÅÇ§ÖÖÈáÇé£¬Íò°ãÃÛÒâ"},
    {"nod", "Ïò", "µãÍ·³ÆÊÇ"},
    {"nudge", "ÓÃÊÖÖâ¶¥", "µÄ·Ê¶Ç×Ó"},
    {"oh", "¶Ô", "Ëµ£º¡°Å¶£¬½´×Ó°¡£¡¡±"},
    {"pad", "ÇáÅÄ", "µÄ¼ç°ò"},
    {"papaya", "ÇÃÁËÇÃ", "µÄÄ¾¹ÏÄÔ´ü"},
    {"pat", "ÇáÇáÅÄÅÄ", "µÄÍ·"},
    {"pinch", "ÓÃÁ¦µÄ°Ñ", "Å¡µÄºÚÇà"},
    {"puke", "¶Ô×Å", "ÍÂ°¡ÍÂ°¡£¬¾İËµÍÂ¶à¼¸´Î¾ÍÏ°¹ßÁË"},
    {"punch", "ºİºİ×áÁË", "Ò»¶Ù"},
    {"pure", "¶Ô", "Â¶³ö´¿ÕæµÄĞ¦Èİ"},
    {"qmarry", "Ïò", "ÓÂ¸ÒµÄ¹òÁËÏÂÀ´:\\n\"ÄãÔ¸Òâ¼Ş¸øÎÒÂğ£¿\"\\n---ÕæÊÇÓÂÆø¿É¼Î°¡"},
    {"report", "ÍµÍµµØ¶Ô", "Ëµ£º¡°±¨¸æÎÒºÃÂğ£¿¡±"},
    {"shrug", "ÎŞÄÎµØÏò", "ËÊÁËËÊ¼ç°ò"},
    {"sigh", "¶Ô", "Ì¾ÁËÒ»¿ÚÆø"},
    {"slap", "Å¾Å¾µÄ°ÍÁË", "Ò»¶Ù¶ú¹â"},
    {"smooch", "ÓµÎÇÖø", ""},
    {"snicker", "ºÙºÙºÙ..µÄ¶Ô", "ÇÔĞ¦"},
    {"sniff", "¶Ô", "àÍÖ®ÒÔ±Ç"},
    {"sorry", "Í´¿ŞÁ÷ÌéµÄÇëÇó", "Ô­ÁÂ"},
    {"spank", "ÓÃ°ÍÕÆ´ò", "µÄÍÎ²¿"},
    {"squeeze", "½ô½ôµØÓµ±§Öø", ""},
    {"thank", "Ïò", "µÀĞ»"},
    {"tickle", "¹¾ß´!¹¾ß´!É¦", "µÄÑ÷"},
    {"waiting", "ÉîÇéµØ¶Ô", "Ëµ£ºÃ¿ÄêÃ¿ÔÂµÄÃ¿Ò»Ìì£¬Ã¿·ÖÃ¿ÃëÎÒ¶¼ÔÚÕâÀïµÈ×ÅÄã"},
    {"wake", "Å¬Á¦µÄÒ¡Ò¡", "£¬ÔÚÆä¶ú±ß´ó½Ğ£º¡°¿ìĞÑĞÑ£¬»á×ÅÁ¹µÄ£¡¡±"},
    {"wave", "¶ÔÖø", "Æ´ÃüµÄÒ¡ÊÖ"},
    {"welcome", "ÈÈÁÒ»¶Ó­", "µÄµ½À´"},
    {"wink", "¶Ô", "ÉñÃØµÄÕ£Õ£ÑÛ¾¦"},
    {"xixi", "ÎûÎûµØ¶Ô", "Ğ¦ÁË¼¸Éù"},
    {"zap", "¶Ô", "·è¿ñµÄ¹¥»÷"},
    {"inn", "Ë«ÑÛ±¥º¬×ÅÀáË®£¬ÎŞ¹¼µÄÍû×Å", ""},
    {"mm", "É«ÃĞÃĞµÄ¶Ô", "ÎÊºÃ£º¡°ÃÀÃ¼ºÃ¡«¡«¡«¡«¡±¡£´óÉ«ÀÇ°¡£¡£¡£¡"},
    {"disapp", "ÕâÏÂÃ»¸ãÍ·À²£¬ÎªÊ²Ã´", "¹ÃÄï¶ÔÎÒÕâ¸öÔìĞÍÍêÈ«Ã»·´Ó¦£¿Ã»°ì·¨£¡"},
    {"miss", "Õæ³ÏµÄÍû×Å", "£ºÎÒÏëÄîÄãÎÒÕæµÄÏëÄîÄãÎÒÌ«--ÏëÄîÄãÁË!ÄãÏà²»ÏàĞÅ?"},
    {"buypig", "Ö¸×Å", "£º¡°Õâ¸öÖíÍ·¸øÎÒÇĞÒ»°ë£¬Ğ»Ğ»£¡¡±"},
    {"rascal", "¶Ô", "´ó½Ğ£º¡°ÄãÕâ¸ö³ôÁ÷Ã¥£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡¡±"},
    {"qifu", "Ğ¡×ìÒ»±â£¬¶Ô", "¿ŞµÀ£º¡°ÄãÆÛ¸ºÎÒ£¬ÄãÆÛ¸ºÎÒ£¡£¡£¡¡±"},
    {"wa", "¶Ô", "´ó½ĞÒ»Éù£º¡°ÍÛÍÛÍÛÍÛÍÛÍÛ¿á±×ÁËÒ®£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡¡±"},
    {"feibang", "à¸£­£­£¡Êì¹éÊì£¬", "ÄãÕâÑùÂÒ½²»°£¬ÎÒÒ»Ñù¿ÉÒÔ¸æÄã»Ù°ù£¬¹ş£¡"},
    {NULL, NULL, NULL}
};

struct action speak_data[] = {
    {"ask", "Ñ¯ÎÊ", NULL},
    {"chant", "¸èËÌ", NULL},
    {"cheer", "ºÈ²É", NULL},
    {"chuckle", "ÇáĞ¦", NULL},
    {"curse", "ÖäÂî", NULL},
    {"demand", "ÒªÇó", NULL},
    {"frown", "õ¾Ã¼", NULL},
    {"groan", "ÉëÒ÷", NULL},
    {"grumble", "·¢ÀÎÉ§", NULL},
    {"hum", "à«à«×ÔÓï", NULL},
    {"moan", "±¯Ì¾", NULL},
    {"notice", "×¢Òâ", NULL},
    {"order", "ÃüÁî", NULL},
    {"ponder", "ÉòË¼", NULL},
    {"pout", "àÙÖø×ìËµ", NULL},
    {"pray", "Æíµ»", NULL},
    {"request", "¿ÒÇó", NULL},
    {"shout", "´ó½Ğ", NULL},
    {"sing", "³ª¸è", NULL},
    {"smile", "Î¢Ğ¦", NULL},
    {"smirk", "¼ÙĞ¦", NULL},
    {"swear", "·¢ÊÄ", NULL},
    {"tease", "³°Ğ¦", NULL},
    {"whimper", "ÎØÑÊµÄËµ", NULL},
    {"yawn", "¹şÇ·Á¬Ìì", NULL},
    {"yell", "´óº°", NULL},
    {NULL, NULL, NULL}
};

struct action condition_data[] = {
    {":D", "ÀÖµÄºÏ²»Â£×ì", NULL},
    {":)", "ÀÖµÄºÏ²»Â£×ì", NULL},
    {":P", "ÀÖµÄºÏ²»Â£×ì", NULL},
    {":(", "ÀÖµÄºÏ²»Â£×ì", NULL},
    {"applaud", "Å¾Å¾Å¾Å¾Å¾Å¾Å¾....", NULL},
    {"blush", "Á³¶¼ºìÁË", NULL},
    {"cough", "¿ÈÁË¼¸Éù", NULL},
    {"faint", "ßÛµ±Ò»Éù£¬ÔÎµ¹ÔÚµØ", NULL},
    {"happy", "µÄÁ³ÉÏÂ¶³öÁËĞÒ¸£µÄ±íÇé£¬²¢Ñ§³Ô±¥ÁËµÄÖíºßºßÁËÆğÀ´", NULL},
    {"lonely", "Ò»¸öÈË×øÔÚ·¿¼äÀï£¬°ÙÎŞÁÄÀµ£¬Ï£ÍûË­À´ÅãÅã¡£¡£¡£¡£", NULL},
    {"luck", "ÍÛ£¡¸£ÆøÀ²£¡", NULL},
    {"puke", "Õæ¶ñĞÄ£¬ÎÒÌıÁË¶¼ÏëÍÂ", NULL},
    {"shake", "Ò¡ÁËÒ¡Í·", NULL},
    {"sleep", "Zzzzzzzzzz£¬ÕæÎŞÁÄ£¬¶¼¿ìË¯ÖøÁË", NULL},
    {"so", "¾Í½´×Ó!!", NULL},
    {"strut", "´óÒ¡´ó°ÚµØ×ß", NULL},
    {"tongue", "ÍÂÁËÍÂÉàÍ·", NULL},
    {"think", "ÍáÖøÍ·ÏëÁËÒ»ÏÂ", NULL},
    {"wawl", "¾ªÌì¶¯µØµÄ¿Ş", NULL},
    {NULL, NULL, NULL}
};

int main()
{
    char t[80];
    int pid;

    init_all();
    if (!loginok) {
	http_fatal("´íÎó£¬ÇëÏÈµÇÂ¼");
	exit(0);
    }
    if (!can_enter_chatroom())
	http_fatal("Äú²»ÄÜ½øÈë" CHAT_SERVER);
    strsncpy(t, getparm("t"), 80);
    pid = atoi(getparm("pid"));
    if (pid == 0)
	reg();
    if (pid > 0) {
	if (kill(pid, 0) != 0) {
	    printf("<script>top.location='about:ÄúÒÑ¾­¶ÏÏß»òÒÑÓĞ1¸ö´°¿Ú½øÈëÁÄÌìÊÒÁË.'</script>");
	    exit(0);
	}
    }
    if (!strcmp(t, "frm"))
	show_frm(pid);
    if (!strcmp(t, "chatsnd"))
	chatsnd(pid);
    if (!strcmp(t, "frame_input"))
	frame_input(pid);
    if (!strcmp(t, "chatrefresh")) {
	test = 1;
	chatrefresh(pid);
    }
    printf("err cmd");
}

int reg()
{
    int n, pid;

    if (pid = fork()) {
	sprintf(genbuf, "bbschat?pid=%d&t=frm", pid);
	redirect(genbuf);
	http_quit();
    }
    for (n = 0; n < 1024; n++)
	close(n);
    pid = getpid();
    agent(pid);
    exit(0);
}

int show_frm(int pid)
{
    printf("<frameset rows=\"0,0,*,48,16\" frameborder=\"0\">\n" "	<frame name=\"hide\" src=\"\">\n" "	<frame name=\"hide2\" src=\"\">\n"
	   /*"  <frame name=\"hide3\" src=\"/chataid.html\">\n" */
	   "	<frame name=\"main\" src=\"\">\n"
	   "	<frame scrolling=\"auto\" marginheight=\"1\" framespacing=\"1\" name=\"input\" src=\"bbschat?t=frame_input&pid=%d\">\n"
	   "	<frame scrolling=\"no\" marginwidth=\"4\" marginheight=\"1\" framespacing=\"1\" name=\"foot\" src=\"bbsfoot\">\n" "	</frameset>\n" "	</html>\n", pid);
    http_quit();
}

int frame_input(int pid)
{
    printf(" <script>\n"
	   "		function r1() {\n"
	   "			top.hide2.location='bbschat?t=chatrefresh&pid=%d';\n"
	   "			setTimeout('r1()', 10000);\n"
	   "		}\n"
	   "		setTimeout('r1()', 500);\n"
	   "	</script>\n"
	   "	<body onload=\"document.form1.in1.focus()\">\n"
	   "	<nobr>\n"
	   "	<form onsubmit=\"add_cc()\" name=\"form1\" action=\"bbschat?pid=%d&t=chatsnd\" method=\"post\" target=\"hide\">\n"
	   "	ÊäÈë: <input name=\"in1\" maxlength=\"60\" size=\"56\">\n"
	   "	<input type=\"submit\" value=\"·¢ËÍ\">\n"
	   "	<script>\n"
	   "		var cc, cc2;\n"
	   "		cc='';\n"
	   "		function add_cc0(x1, x) {\n"
	   "			cc2=x1;\n"
	   "			cc=x;\n"
	   "		}\n"
	   "		function do_quit() {\n"
	   "			if(confirm('ÄúÕæµÄÒªÍË³öÁËÂğ£¿')) {\n"
	   "				form1.in1.value='/b';\n"
	   "				form1.submit();\n"
	   "			}\n"
	   "		}\n"
	   "		function do_help() {\n"
	   "			open('/chathelp.html', '_blank', \n"
	   "			'toolbar=yes,location=no,status=no,menubar=no,scrollbar=auto,resizable=yes,width=620,height=400');\n"
	   "		}\n"
	   "		function do_alias(x) {\n"
	   "			form1.in1.value=x;\n"
	   "			form1.submit();\n"
	   "		}\n"
	   "		function do_room() {\n"
	   "			xx=prompt('ÇëÊäÈë°üÏáÃû³Æ','');\n"
	   "			if(xx=='' || xx==null) return;\n"
	   "			form1.in1.value='/j '+ xx;\n"
	   "			form1.submit();\n"
	   "		}\n"
	   "		function do_user() {\n"
	   "			form1.in1.value='/l';\n"
	   "			form1.submit();\n"
	   "		}\n"
	   "		function do_r() {\n"
	   "			form1.in1.value='/r';\n"
	   "			form1.submit();\n"
	   "		}\n"
	   "		function do_w() {\n"
	   "			form1.in1.value='/w';\n"
	   "			form1.submit();\n"
	   "		}\n"
	   "		function do_msg() {\n"
	   "			xx=prompt('¸øË­¶ªĞ¡Ö½Ìõ','');\n"
	   "			if(xx=='' || xx==null) return;\n"
	   "			yy=prompt('Ê²Ã´ÄÚÈİ','');\n"
	   "			if(yy=='' || xx==null) return;\n"
	   "			form1.in1.value='/m '+xx+' '+yy;\n"
	   "			form1.submit();\n"
	   "		}\n"
	   "		function do_n() {\n"
	   "			xx=prompt('ÄãÏë¸Ä³ÉÊ²Ã´Ãû×Ö?','');\n"
	   "			if(xx=='' || xx==null) return;\n"
	   "			form1.in1.value='/n '+xx;\n"
	   "			form1.submit();\n"
	   "		}\n"
	   "		function do_pic() {\n"
	   "			xx=prompt('ÇëÊäÈëÍ¼Æ¬µÄURLµØÖ·:','http://');\n"
	   "			if(xx=='http://' || xx=='' || xx==null) return;\n"
	   "			form1.in1.value='<img src=\"'+xx+'\">';\n"
	   "			form1.submit();\n"
	   "		}\n"
	   "	</script>\n" "	<select onChange=\"do_alias(this.options[this.selectedIndex].value);this.selectedIndex=0;\">\n" "        <option value=\" \" selected>ÁÄÌì¶¯×÷</option>\n"
	   /*"        <option value=\"//hehe\">ºÇºÇµÄÉµĞ¦</option>\n" */
	   "	<option value=\"//faint\">ÒªÔÎµ¹ÁË</option>\n"
	   "	<option value=\"//sleep\">ÒªË¯×ÅÁË</option>\n"
	   "	<option value=\"//:D\">ÀÖ×Ì×ÌµÄ</option>\n"
	   "	<option value=\"//nod\">µãÍ·</option>\n"
	   "	<option value=\"//so\">¾ÍÕâÑù</option>\n"
	   "	<option value=\"//shake\">Ò¡Ò¡Í·</option>\n"
	   "	<option value=\"//luck\">ÕæĞÒÔË°¡</option>\n"
	   "	<option value=\"//tongue\">ÍÂÍÂÉàÍ·</option>\n"
	   "	<option value=\"//blush\">Á³ºìÁË</option>\n"
	   "	<option value=\"//applaud\">ÈÈÁÒ¹ÄÕÆ</option>\n"
	   "	<option value=\"//cough\">¿ÈËÔÒ»ÏÂ</option>\n"
	   "	<option value=\"//happy\">ĞÒ¸£</option>\n"
	   "	<option value=\"//hungry\">¶Ç×Ó¶öÁË</option>\n"
	   "	<option value=\"//strut\">´óÒ¡´ó°Ú</option>\n"
	   "	<option value=\"//miss\">ÏëÄî</option>\n"
	   "	<option value=\"//think\">ÏëÒ»Ïë</option>\n"
	   "	<option value=\"//?\">ÒÉ»ó²»ÒÑ</option>\n"
	   "	<option value=\"//bearbug\">ÈÈÇéÓµ±§</option>\n"
	   "	<option value=\"//bless\">×£¸£</option>\n"
	   "	<option value=\"//bow\">¾Ï¹ª</option>\n"
	   "        <option value=\"//caress\">¸§Ãş</option>\n"
	   "        <option value=\"//cringe\">ÆóÇó¿íË¡</option>\n"
	   "        <option value=\"//cry\">·ÅÉù´ó¿Ş</option>\n"
	   "        <option value=\"//comfort\">°²Î¿Ò»ÏÂ</option>\n"
	   "	<option value=\"//clap\">ÈÈÁÒ¹ÄÕÆ</option>\n"
	   "        <option value=\"//dance\">ôæôæÆğÎè</option>\n"
	   "    	<option value=\"//drivel\">Á÷¿ÚË®</option>\n"
	   "    	<option value=\"//wave\">Ò¡ÊÖ</option>\n"
	   "    	<option value=\"//bye\">ÔÙ¼û</option>\n"
	   "  	<option value=\"//giggle\">´ôĞ¦</option>\n"
	   "    	<option value=\"//grin\">Ğ°¶ñµØĞ¦</option>\n"
	   "      	<option value=\"//growl\">´óÉùÅØÏø</option>\n"
	   "        </select>\n"
	   "	<select name=\"ccc\" onChange=\"add_cc0(this, this.options[this.selectedIndex].value)\">\n"
	   "	<option value=\"\" selected>°×É«</option>\n"
	   "	<option value=\"%s\"><font color=\"green\">ºìÉ«</font></option>\n"
	   "	<option value=\"%s\"><font color=\"red\">ÂÌÉ«</font></option>\n"
	   "        <option value=\"%s\"><font color=\"blue\">À¶É«</font></option>\n"
	   "        <option value=\"%s\"><font color=\"blue\">ÌìÀ¶</font></option>\n"
	   "        <option value=\"%s\"><font color=\"yellow\">»ÆÉ«</font></option>\n"
	   "        <option value=\"%s\"><font color=\"red\">Æ·ºì</font></option>\n"
	   "	<option value=\"%s\">´ó×Ö</option>\n"
	   "	</select>\n"
	   "	<select onChange=\"do_func(this.selectedIndex);this.selectedIndex=0;\">\n"
	   " 	<option selected>ÁÄÌìÊÒ¹¦ÄÜ</option>\n"
	   "	<option>½øÈë" CHAT_ROOM_NAME "</option>\n"
	   "	<option>²é¿´" CHAT_ROOM_NAME "Ãû³Æ</option>\n"
	   "	<option>±¾" CHAT_ROOM_NAME "ÓĞË­</option>\n"
	   "	<option>¿´ÓĞË­ÔÚÏß</option>\n"
	   "	<option>¶ªĞ¡Ö½Ìõ</option>\n"
	   "	<option>¸ÄÁÄÌì´úºÅ</option>\n"
	   "	<option>ÌùÍ¼Æ¬</option>\n"
	   "	<option>Çå³ıÆÁÄ»</option>\n"
	   "	<option>±³¾°·´É«</option>\n"
	   "	<option>Àë¿ªÁÄÌìÊÒ</option>\n"
	   "        </select>\n"
	   "	<br>\n"
	   "	[<a href=\"javascript:do_quit()\">Àë¿ª"
	   CHAT_SERVER "</a>] \n"
	   "	[<a href=\"/chathelp.html\" target=\"_blank\">ÁÄÌìÊÒ°ïÖú</a>]\n"
	   "	<script>\n"
	   "	function do_func(n) {\n"
	   "		if(n==0) return;\n"
	   "		if(n==1) return do_room();\n"
	   "		if(n==2) return do_r();\n"
	   "		if(n==3) return do_w();\n"
	   "		if(n==4) return do_user();\n"
	   "		if(n==5) return do_msg();\n"
	   "		if(n==6) return do_n();\n"
	   "		if(n==7) return do_pic();\n"
	   "		if(n==8) return do_c();\n"
	   "		if(n==9) return do_css2();\n"
	   "		if(n==10) return do_quit();\n"
	   "	}\n"
	   "	var css1;\n"
	   "	css1='/bbschat.css';\n"
	   "	function do_c() {\n"
	   "		top.main.document.close();\n"
	   "                top.main.document.writeln('<link rel=\"stylesheet\" type=\"text/css\" href=\"'+css1+'\"><body><pre><font class=\"c37\">');\n"
	   "	}\n"
	   "	function do_css2() {\n"
	   "		if(css1=='bbschat.css')\n"
	   "			css1='/bbschat2.css';\n"
	   "		else\n"
	   "			css1='/bbschat.css';\n"
	   "		top.main.document.writeln('<link rel=\"stylesheet\" type=\"text/css\" href=\"'+css1+'\"><body><pre><font class=\"c37\">');\n"
	   "	}\n"
	   "	function add_cc() {\n"
	   "	 	xxx=form1.in1.value;\n"
	   "		if(xxx=='/h' || xxx=='//') {\n"
	   "			do_help();\n"
	   "			form1.in1.value='';\n"
	   "			return; \n"
	   "		}\n"
	   "		if(xxx=='/c') {\n"
	   "			do_c();\n"
	   "			form1.in1.value='';\n"
	   "			return;\n"
	   "		}\n"
	   "		if(xxx=='') return;\n"
	   " 		if(xxx.indexOf('/')<0) {\n"
	   " 			form1.in1.value=cc+xxx;\n"
	   " 		}\n"
	   " 		if(cc=='/I') {\n"
	   " 			cc='';\n"
	   " 			cc2.selectedIndex=0;\n" " 		}\n" " 	}\n" "	</script>\n" "	</form></body>\n", pid, pid, "%R", "%G", "%B", "%C", "%Y", "%M", "%I");
    http_quit();
}

int chatsnd(int pid)
{
    char in1[255], filename[256];
    FILE *fp;

    strsncpy(in1, getparm("in1"), 60);
    sprintf(filename, "tmp/%d.in", pid);
    fp = fopen(filename, "a");
    fprintf(fp, "%s\n\n", in1);
    fclose(fp);
    chatrefresh(pid);
}

char *cco(char *s)
{
    static char buf[512];
    char *p = buf, co[20];

    bzero(buf, 512);
    while (s[0]) {
	if (s[0] != '%') {
	    p[0] = s[0];
	    p++;
	    s++;
	    continue;
	}
	bzero(co, 20);
	if (!strncmp(s, "%R", 2))
	    strcpy(co, "[31m");
	if (!strncmp(s, "%G", 2))
	    strcpy(co, "[32m");
	if (!strncmp(s, "%B", 2))
	    strcpy(co, "[34m");
	if (!strncmp(s, "%C", 2))
	    strcpy(co, "[36m");
	if (!strncmp(s, "%Y", 2))
	    strcpy(co, "[33m");
	if (!strncmp(s, "%M", 2))
	    strcpy(co, "[35m");
	if (!strncmp(s, "%N", 2))
	    strcpy(co, "[0m");
	if (!strncmp(s, "%W", 2))
	    strcpy(co, "[37m");
	if (!strncmp(s, "%I", 2))
	    strcpy(co, "[99m");
	if (co[0]) {
	    strncpy(p, co, strlen(co));
	    p += strlen(co);
	    s += 2;
	    continue;
	}
	p[0] = s[0];
	p++;
	s++;
    }
    return buf;
}

int chatrefresh(int pid)
{
    char filename[256], tmp[256];
    int t1;
    FILE *fp;

    kill(pid, SIGINT);
    usleep(150000);
    if (kill(pid, 0) != 0) {
	printf("<script>top.location='javascript:close()';</script>");
	exit(0);
    }
    sprintf(filename, "tmp/%d.out", pid);
    t1 = time(0);
    while (abs(t1 - time(0)) < 8 && !file_exist(filename)) {
	sleep(1);
	continue;
    }
    fp = fopen(filename, "r");
    if (fp) {
	while (1) {
	    int i;
	    char buf2[512];

	    if (fgets(buf2, 255, fp) == NULL)
		break;
	    sprintf(genbuf, "%s", cco(buf2));
	    for (i = 0; genbuf[i]; i++) {
		if (genbuf[i] == 10 || genbuf[i] == 13)
		    genbuf[i] = 0;
	    }
	    if (!strncmp(genbuf, "/init", 5)) {
		printf("<script>\n");
		printf("top.main.document.write('");
		printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"/bbschat.css\"><body id=\"body1\" bgColor=\"black\"><pre><img src=\"/pic001.gif\">");
		printf("');");
		printf("\n</script>\n");
		continue;
	    }
	    if (!strncmp(genbuf, "/t", 2)) {
		int i;

		printf("<script>top.document.title='%s%s--»°Ìâ: ", BBS_FULL_NAME, CHAT_SERVER);
		hprintf("%s", genbuf + 2);
		printf("'</script>");
		sprintf(buf2, "±¾%sµÄ»°ÌâÊÇ: [[1;33m%s[37m]", CHAT_ROOM_NAME, genbuf + 2);
		strcpy(genbuf, buf2);
	    }
	    if (!strncmp(genbuf, "/r", 2)) {
		sprintf(buf2, "±¾%sµÄÃû³ÆÊÇ: [[1;33m%s[37m]", CHAT_ROOM_NAME, genbuf + 2);
		strcpy(genbuf, buf2);
	    }
	    if (!strncmp(genbuf, "/", 1)) {
		genbuf[0] = '>';
		genbuf[1] = '>';
	    }
	    for (i = 0; i < strlen(genbuf); i++) {
		if (genbuf[i] == 10 || genbuf[i] == 13)
		    genbuf[i] = 0;
		if (genbuf[i] == 34)
		    genbuf[i] = 39;
	    }
	    printf("<script>\n");
	    printf("top.main.document.writeln('");
	    hhprintf("%s", genbuf);
	    printf(" <font class=\"c37\">");
	    printf("');");
	    printf("top.main.scrollBy(0, 99999);\n");
	    if (test == 0)
		printf("top.input.form1.in1.value='';\n");
	    printf("</script>\n");
	}
    }
    unlink(filename);
    printf("<br>");
    http_quit();
}

void foo()
{
    FILE *fp;
    char filename[80], buf[256];

    sprintf(filename, "tmp/%d.in", getpid());
    fp = fopen(filename, "r");
    if (fp) {
	while (1) {
	    if (fgets(buf, 250, fp) == NULL)
		break;
	    write(chat_fd, buf, strlen(buf));
	}
	fclose(fp);
    }
    unlink(filename);
    alarm(60);
}

void abort_chat()
{
    int pid = getpid();
    char filename[200];

    sprintf(filename, "tmp/%d.out", pid);
    unlink(filename);
    sprintf(filename, "tmp/%d.in", pid);
    unlink(filename);
    exit(0);
}

int agent(int pid)
{
    int i, num;
    FILE *fp;
    char filename[80];
    struct sockaddr_in blah;

    sprintf(filename, "tmp/%d.out", pid);
    bzero((char *) &blah, sizeof(blah));
    blah.sin_family = AF_INET;
    blah.sin_addr.s_addr = inet_addr("127.0.0.1");
    blah.sin_port = htons(CHATPORT3);
    chat_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect(chat_fd, (struct sockaddr *) &blah, 16) < 0)
	return;
    signal(SIGINT, foo);
    signal(SIGALRM, abort_chat);
    alarm(60);
    sprintf(genbuf, "/! %d %d %s %s %d\n", u_info->uid, currentuser->userlevel, currentuser->userid, currentuser->userid, 0);
    write(chat_fd, genbuf, strlen(genbuf));
    read(chat_fd, genbuf, 2);
    if (!strcasecmp(genbuf, "OK"))
	exit(0);
    fp = fopen(filename, "w");
    fprintf(fp, "/init\n");
    fclose(fp);
    while (1) {
	num = read(chat_fd, genbuf, 2048);
	if (num <= 0)
	    break;
	for (i = 0; i < num; i++)
	    if (genbuf[i] == 0)
		genbuf[i] = 10;
	genbuf[num] = 0;
	fp = fopen(filename, "a");
	fprintf(fp, "%s", genbuf);
	fclose(fp);
    }
    abort_chat();
}
