/*  ÀÆƒæ«Âª™BBS ◊®”√∞Ê√Ê π”√«ÈøˆÕ≥º∆≥Ã–Ú  modified from usage.c   . by binxun*/
#include <time.h>
#include <stdio.h>
#include "bbs.h"
#include "config.h"

#define MAX_SEC_BOARD 100

struct binfo {
    char boardname[15];
    char expname[25];
    int times;
    int sum;
} st[MAXBOARD];//∆‰À˚∞Ê√Ê,≥˝ø™œ¬√Êµƒ∑÷«¯,0,1∑÷«¯Õ‚.


/* sec 2,9,A,B,C ∑÷ø™Õ≥º∆ */
struct binfo sec_board[5][MAX_SEC_BOARD];
int sec_board_num[5]={0};

int numboards = 0;

int brd_cmp(const void* b1, const void* a1)
{
    struct binfo* a ,*b;
    a = (struct binfo*)a1;
    b = (struct binfo*)b1;

    if (a->times != b->times)
        return (a->times - b->times);
    return a->sum - b->sum;
}

int total_cmp(const void* b1, const void* a1)
{
    struct binfo *a, *b;
    
    a = (struct binfo*)a1;
    b = (struct binfo*)b1;
    if (a->sum != b->sum)
        return (a->sum - b->sum);
    return a->times - b->times;
}

int average_cmp(const void* b1, const void* a1)
{
    struct binfo *a, *b;
    int a_ave, b_ave;
    
    a = (struct binfo*)a1;
    b = (struct binfo*)b1;	 
    if (a->times)
        a_ave = a->sum / a->times;
    else
        a_ave = 0;
    if (b->times)
        b_ave = b->sum / b->times;
    else
        b_ave = 0;

    if (a_ave != b_ave)
        return (a_ave - b_ave);
    return a->sum - b->sum;
}

int record_data(board, sec)
    char *board;
    int sec;
{
    int i,j;

    for (i = 0; i < numboards; i++) {
        if (!strcmp(st[i].boardname, board)) {
            st[i].times++;
            st[i].sum += sec;
            return;
        }
    }
	for (i = 0; i < 5;i++)
	    for(j = 0;j < sec_board_num[i];j++){
            if (!strcmp(sec_board[i][j].boardname, board)) {
            sec_board[i][j].times++;
            sec_board[i][j].sum += sec;
            return;
		}
	}
    return;
}

int fillsecboard(struct boardheader* fptr,int i)
{
    if(i < 0 || i >= 5) return 0;
    strcpy(sec_board[i][sec_board_num[i]].boardname,fptr->filename);
	strcpy(sec_board[i][sec_board_num[i]].expname, fptr->title+13);
	sec_board[i][sec_board_num[i]].times = 0;
	sec_board[i][sec_board_num[i]].sum = 0;
	sec_board_num[i]++;
	
	if(sec_board_num[i] >= MAX_SEC_BOARD -1){
	    printf("∑÷«¯ƒ⁄µƒ∞Ê√ÊÃ´∂‡,«Îµ˜’˚ MAX_SEC_BOARD ≤Œ ˝!");
		exit(0);
	}
}

int fillbcache(struct boardheader *fptr,void* arg)
{
    if (numboards >= MAXBOARD)
        return 0;
    if (check_see_perm(NULL, fptr)==0|| strlen(fptr->filename) == 0)
        return;

    switch(fptr->title[0])
	{
	    case '0':
	    case '1':
		    return 0;
			break;
        case '2':
            fillsecboard(fptr,0);
		    break;
        case '9':
            fillsecboard(fptr,1);
		    break;
	    case 'A':
            fillsecboard(fptr,2);
		    break;
	    case 'B':
            fillsecboard(fptr,3);
		    break;
		case 'C':
            fillsecboard(fptr,4);
		    break;
        default:
            strcpy(st[numboards].boardname, fptr->filename);
            strcpy(st[numboards].expname, fptr->title + 13);
            st[numboards].times = 0;
            st[numboards].sum = 0;
            numboards++;
		    break;
	}
    return 0;
}

int fillboard()
{
    int i = 0;
    resolve_boards();
    resolve_ucache();
    /*apply_record(BOARDS, (APPLY_FUNC_ARG)fillbcache, sizeof(struct boardheader), NULL, 0,false);*/
    apply_boards(fillbcache,NULL);
    for(i = 0;i < 5; i++) 	{
    	if(0 == sec_board_num[i] ){
    		printf(" ∏√∑÷«¯÷–√ª”–∞Ê√Ê! ∑÷«¯ %d",i);
    		exit(0);
    		}
    	}
}

char *timetostr(i)
    int i;
{
    static char str[30];
    int minute, sec, hour;

    minute = (i / 60);
    hour = minute / 60;
    minute = minute % 60;
    sec = i & 60;
    sprintf(str, "%2d:%2d:%2d", hour, minute, sec);
    return str;
}

void save_useboard_xml(int brdcount, struct binfo *bi)
{
    int i;
    FILE *fp;
    char xmlfile[STRLEN];

    snprintf(xmlfile, sizeof(xmlfile), BBSHOME "/0Announce/bbslists/board.xml");
    if ((fp = fopen(xmlfile, "w")) == NULL)
        return;
    fprintf(fp, "<?xml version=\"1.0\" encoding=\"GB2312\"?>\n");
    fprintf(fp, "<BoardList Desc=\"Ã÷¬€«¯ π”√◊¥øˆÕ≥º∆\">\n");
    for (i = 0; i < brdcount; i++) {
        fprintf(fp, "<Board>\n");
        fprintf(fp, "<EnglishName>%s</EnglishName>\n", bi[i].boardname);
        fprintf(fp, "<ChineseName>%s</ChineseName>\n", bi[i].expname);
        fprintf(fp, "<VisitTimes>%ld</VisitTimes>\n", bi[i].times);
        fprintf(fp, "<StayTime>%ld</StayTime>\n", bi[i].sum);
        fprintf(fp, "</Board>\n");
    }
    fprintf(fp, "</BoardList>\n");
    fclose(fp);
}

main(argc, argv)
    char *argv[];
{
    char *progmode;
    FILE *fp;
    FILE *op, *op1, *op2;
    char buf[256], buf1[256], buf2[256], *p, bname[20];
    char *q;                    //added by Czz 010614
    char date[80];
    int mode;
    int c[3];
    int max[3];
    unsigned int ave[3];
    int now, sec;
    int i, j, jj,k;
    char *blk[10] = {
        /* ∑ΩøÚÃ´ƒ—ø¥¡À modified by Czz */
//      "  ","  ", "  ", "  ", "  ",
        "  ", "®á", "®Ü", "®Ö", "®Ñ",
//      "°ı","°ı", "°ı", "°ı", "°ı",
        "®É", "®Ç", "®Å", "®Ä", "®Ä",
        /* modified end */
    };

    chdir(BBSHOME);
    if (argc <= 1)
        exit(0);
    mode = atoi(argv[1]);
    if (mode == 1) {
        strcpy(buf, BBSHOME "/0Announce/groups/system.faq/sys_discuss/boardstats/board2");
        strcpy(buf1, BBSHOME "/0Announce/groups/system.faq/sys_discuss/boardstats/totaltime");
        strcpy(buf2, BBSHOME "/0Announce/groups/system.faq/sys_discuss/boardstats/averagetime");
    } else {
        strcpy(buf, BBSHOME "/0Announce/groups/system.faq/sys_discuss/boardstats/board1");
     }
    if ((fp = fopen(BBSHOME "/boardusage.log", "r")) == NULL) {
        printf("cann't open boardusage.log\n");
        return 1;
    }
    if (mode == 1) {
        if ((op = fopen(buf, "w")) == NULL || (op1 = fopen(buf1, "w")) == NULL || (op2 = fopen(buf2, "w")) == NULL) {
            printf("Can't Write file\n");
            return 1;
        }
    } else if ((op = fopen(buf, "w")) == NULL) {
        printf("Can't Write file\n");
        return 1;
    }


    fillboard();
    now = time(0);
    sprintf(date, "%6.6s", Ctime(now) + 4);
    printf("%6.6s", date);
     /**/ while (fgets(buf, 256, fp)) {
        if (strlen(buf) < 57)
            continue;
        if (p = (char *) strstr(buf, "Stay: ")) {
            q = p - 21;
            q = strtok(q, " ");
            strcpy(bname, q);
            sec = atoi(p + 6);
        }
        record_data(bname, sec);
    }
    fclose(fp);
    qsort(st, numboards, sizeof(st[0]), brd_cmp);
	for(i = 0;i < 5; i++){
	    qsort(sec_board[i],sec_board_num[i], sizeof(sec_board[i][0]),brd_cmp);
    }
     /**/ ave[0] = 0;
    ave[1] = 0;
    ave[2] = 0;
    max[1] = 0;
    max[0] = 0;
    max[2] = 0;
    for (i = 0; i < numboards; i++) {
        ave[0] += st[i].times;
        ave[1] += st[i].sum;
        ave[2] += st[i].times == 0 ? 0 : st[i].sum / st[i].times;
        if (max[0] < st[i].times) {
            max[0] = st[i].times;
        }
        if (max[1] < st[i].sum) {
            max[1] = st[i].sum;
        }
        if (max[2] < (st[i].times == 0 ? 0 : st[i].sum / st[i].times)) {
            max[2] = (st[i].times == 0 ? 0 : st[i].sum / st[i].times);
        }
    }
    c[0] = max[0] / 30 + 1;
    c[1] = max[1] / 30 + 1;
    c[2] = max[2] / 30 + 1;
    numboards++;
    st[numboards - 1].times = ave[0] / numboards;
    st[numboards - 1].sum = ave[1] / numboards;
    strcpy(st[numboards - 1].boardname, "Average");
    strcpy(st[numboards - 1].expname, "◊‹∆Ωæ˘");

	//∑÷«¯Õ≥º∆
	for(i = 0; i < 5; i++){
        ave[0] = 0;
		ave[1] = 0;
		ave[2] = 0;
		max[1] = 0;
		max[0] = 0;
		max[2] = 0;
		for (j = 0; j < sec_board_num[i]; j++) {
			ave[0] += sec_board[i][j].times;
			ave[1] += sec_board[i][j].sum;
			ave[2] += sec_board[i][j].times == 0 ? 0 : sec_board[i][j].sum / sec_board[i][j].times;
			if (max[0] < sec_board[i][j].times) max[0] = sec_board[i][j].times;
			if (max[1] < sec_board[i][j].sum) max[1] = sec_board[i][j].sum;

			if (max[2] < sec_board[i][j].times == 0 ? 0 : sec_board[i][j].sum / sec_board[i][j].times) {
				max[2] = (sec_board[i][j].times == 0 ? 0 : sec_board[i][j].sum / sec_board[i][j].times);
			}
		}
		c[0] = max[0] / 30 + 1;
		c[1] = max[1] / 30 + 1;
		c[2] = max[2] / 30 + 1;
		sec_board_num[i]++;
		sec_board[i][sec_board_num[i]-1].times = ave[0] / (sec_board_num[i] -1);
		sec_board[i][sec_board_num[i]-1].sum = ave[1] / (sec_board_num[i] -1);
		strcpy(sec_board[i][sec_board_num[i]-1].boardname, "Average");
		strcpy(sec_board[i][sec_board_num[i]-1].expname, "◊‹∆Ωæ˘");
	}

    if (mode == 1) {
        fprintf(op, "√˚¥Œ %-15.15s%-25.25s %5s %8s %10s\n", "Ã÷¬€«¯√˚≥∆", "÷–Œƒ– ˆ", "»À¥Œ", "¿€ª˝ ±º‰", "∆Ωæ˘ ±º‰");
    } else {
        fprintf(op, "      [37m1 [m[34m%2s[37m= %d (◊‹»À¥Œ) [37m1 [m[32m%2s[37m= %s (¿€ª˝◊‹ ± ˝) [37m1 [m[31m%2s[37m= %d √Î(∆Ωæ˘ ± ˝)\n\n",
                blk[9], c[0], blk[9], timetostr(c[1]), blk[9], c[2]);
    }

    for (i = 0; i < numboards; i++) {
        if (mode == 1) {
            /* generate 0Announce/bbslists/board2 file */
            fprintf(op, "%4d[m %-15.15s%-25.25s %5d %-.8s %10d\n", i + 1, st[i].boardname, st[i].expname, st[i].times, timetostr(st[i].sum), st[i].times == 0 ? 0 : st[i].sum / st[i].times);
        } else {
            fprintf(op, "      [37mµ⁄[31m%3d [37m√˚ Ã÷¬€«¯√˚≥∆£∫[31m%s [35m%s[m\n", i + 1, st[i].boardname, st[i].expname);
            fprintf(op, "[37m    ©∞°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™\n");
            fprintf(op, "[37m»À¥Œ©¶[m[34m");
            for (j = 0; j < st[i].times / c[0]; j++) {
                fprintf(op, "%2s", blk[9]);
            }
            fprintf(op, "%2s [37m%d[m\n", blk[(st[i].times % c[0]) * 10 / c[0]], st[i].times);
            fprintf(op, "[1;37m ±º‰©¶[m[32m");
            for (j = 0; j < st[i].sum / c[1]; j++) {
                fprintf(op, "%2s", blk[9]);
            }
            fprintf(op, "%2s [37m%s[m\n", blk[(st[i].sum % c[1]) * 10 / c[1]], timetostr(st[i].sum));
            j = st[i].times == 0 ? 0 : st[i].sum / st[i].times;
            fprintf(op, "[37m∆Ωæ˘©¶[m[31m");
            for (k = 0; k < j / c[2]; k++) {
                fprintf(op, "%2s", blk[9]);
            }
            fprintf(op, "%2s [37m%s[m\n", blk[(j % c[2]) * 10 / c[2]], timetostr(j));
            fprintf(op, "[37m    ©∏°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™[m\n\n");
        }
    }

	for(i = 0;i < 5; i ++){
	    if(mode == 1){
		fprintf(op,"\n");
		fprintf(op, "√˚¥Œ %-15.15s%-25.25s %5s %8s %10s\n", "Ã÷¬€«¯√˚≥∆", "÷–Œƒ– ˆ", "»À¥Œ", "¿€ª˝ ±º‰", "∆Ωæ˘ ±º‰");
		for(j = 0;j < sec_board_num[i]; j++)
                fprintf(op, "%4d[m %-15.15s%-25.25s %5d %-.8s %10d\n", j + 1, sec_board[i][j].boardname, sec_board[i][j].expname,
				    sec_board[i][j].times, timetostr(sec_board[i][j].sum), sec_board[i][j].times == 0 ? 0 : sec_board[i][j].sum / sec_board[i][j].times);
		}else{
		
                fprintf(op,"\n∑÷«¯Õ≥º∆:\n");
		    //∑÷«¯¡–±Ì
            for(j = 0;j < sec_board_num[i]; j++)
			{

                fprintf(op, "      [37mµ⁄[31m%3d [37m√˚ Ã÷¬€«¯√˚≥∆£∫[31m%s [35m%s[m\n", j + 1, sec_board[i][j].boardname, sec_board[i][j].expname);
				fprintf(op, "[37m    ©∞°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™\n");
				fprintf(op, "[37m»À¥Œ©¶[m[34m");
				for (jj = 0; jj < sec_board[i][j].times / c[0]; jj++) {
					fprintf(op, "%2s", blk[9]);
				}
				fprintf(op, "%2s [37m%d[m\n", blk[(sec_board[i][j].times % c[0]) * 10 / c[0]], sec_board[i][j].times);
				fprintf(op, "[1;37m ±º‰©¶[m[32m");
				for (jj = 0; jj < sec_board[i][j].sum / c[1]; jj++) {
					fprintf(op, "%2s", blk[9]);
				}
				fprintf(op, "%2s [37m%s[m\n", blk[(sec_board[i][j].sum % c[1]) * 10 / c[1]], timetostr(sec_board[i][j].sum));
				jj = sec_board[i][j].times == 0 ? 0 : sec_board[i][j].sum / sec_board[i][j].times;
				fprintf(op, "[37m∆Ωæ˘©¶[m[31m");
				for (k = 0; k < jj / c[2]; k++) {
					fprintf(op, "%2s", blk[9]);
				}
				fprintf(op, "%2s [37m%s[m\n", blk[(j % c[2]) * 10 / c[2]], timetostr(j));
				fprintf(op, "[37m    ©∏°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™[m\n\n");

			}
        }
	}
    fclose(op);

    /* generate boards usage result in xml format */
    if (mode == 1)
        save_useboard_xml(numboards, st);   //‘› ±≤ªπ‹xmlµƒ
    if (mode == 1) {
        qsort(st, numboards - 1, sizeof(st[0]), total_cmp);
        fprintf(op1, "√˚¥Œ %-15.15s%-25.25s %8s %5s %10s\n", "Ã÷¬€«¯√˚≥∆", "÷–Œƒ– ˆ", "¿€ª˝ ±º‰", "»À¥Œ", "∆Ωæ˘ ±º‰");
        for (i = 0; i < numboards; i++)
            fprintf(op1, "%4d %-15.15s%-25.25s %-.8s %5d %10d\n", i + 1, st[i].boardname, st[i].expname, timetostr(st[i].sum), st[i].times, st[i].times == 0 ? 0 : st[i].sum / st[i].times);

        //∑÷«¯¡–±Ì
		for(i = 0;i < 5; i++){
            qsort(sec_board[i], sec_board_num[i] - 1, sizeof(sec_board[i][0]), total_cmp);
			fprintf(op1,"\n");
            fprintf(op1, "√˚¥Œ %-15.15s%-25.25s %8s %5s %10s\n", "Ã÷¬€«¯√˚≥∆", "÷–Œƒ– ˆ", "¿€ª˝ ±º‰", "»À¥Œ", "∆Ωæ˘ ±º‰");
            for(j = 0; j < sec_board_num[i]; j++)
                fprintf(op1, "%4d %-15.15s%-25.25s %-.8s %5d %10d\n", j + 1, sec_board[i][j].boardname, sec_board[i][j].expname,
			        timetostr(sec_board[i][j].sum), sec_board[i][j].times,sec_board[i][j].times == 0 ? 0 :sec_board[i][j].sum / sec_board[i][j].times);
		}
        fclose(op1);

		qsort(st, numboards - 1, sizeof(st[0]), average_cmp);
        fprintf(op2, "√˚¥Œ %-15.15s%-25.25s %10s %5s %8s\n", "Ã÷¬€«¯√˚≥∆", "÷–Œƒ– ˆ", "∆Ωæ˘ ±º‰", "¿€ª˝ ±º‰", "»À¥Œ");
        for (i = 0; i < numboards; i++)
            fprintf(op2, "%4d %-15.15s%-25.25s %10d %-.8s %5d\n", i + 1, st[i].boardname, st[i].expname, st[i].times == 0 ? 0 : st[i].sum / st[i].times, timetostr(st[i].sum), st[i].times);

        //∑÷«¯¡–±Ì
		for(i = 0;i < 5; i++){
            qsort(sec_board[i], sec_board_num[i] - 1, sizeof(sec_board[i][0]), average_cmp);
			fprintf(op2,"\n");
            fprintf(op2, "√˚¥Œ %-15.15s%-25.25s %10s %5s %8s\n", "Ã÷¬€«¯√˚≥∆", "÷–Œƒ– ˆ", "∆Ωæ˘ ±º‰", "¿€ª˝ ±º‰", "»À¥Œ");
            for (j = 0; j < sec_board_num[i]; j++)
                fprintf(op2, "%4d %-15.15s%-25.25s %10d %-.8s %5d\n", j + 1, sec_board[i][j].boardname, sec_board[i][j].expname,
				    sec_board[i][j].times == 0 ? 0 : sec_board[i][j].sum / sec_board[i][j].times, timetostr(sec_board[i][j].sum), sec_board[i][j].times);
		}
        fclose(op2);
    }
}

