#include "bbslib.h"

int main() {
	char *board, buf[80], *board1, *title;
	int i, total=0;
	bcache_t *bc;

	init_all(); 
	board=nohtml(getparm("board"));
	bc = getbcacheaddr();
	if(board[0]==0) {
		printf("%s -- 选择讨论区<hr color=green>\n", BBSNAME);
		printf("<form action=bbssel>\n");
		printf("讨论区名称: <input type=text name=board>");
		printf(" <input type=submit value=确定>");
		printf("</form>\n");
		http_quit();
	}
	else
	{
		for(i=0; i<MAXBOARD; i++) {
			board1=bc[i].filename;
			if(!has_read_perm(currentuser, board1)) continue;
			if(!strcasecmp(board, board1)) {
				sprintf(buf, "bbsdoc?board=%s", board1);
				redirect(buf);
				http_quit();
			}
		}
		printf("%s -- 选择讨论区<hr color=green>\n", BBSNAME);
		printf("找不到这个讨论区, ", board);
		printf("标题中含有'%s'的讨论区有: <br><br>\n", board);
		printf("<table>");
		for(i=0; i<MAXBOARD; i++) {
			board1=bc[i].filename;
			title=bc[i].title;
			if(!has_read_perm(currentuser, board1)) continue;
			if(strcasestr(board1, board) || strcasestr(title, board)) {
				total++;
				printf("<tr><td>%d", total);
				printf("<td><a href=bbsdoc?board=%s>%s</a><td>%s<br>\n",
					board1, board1, title+7);
			}
		}
		printf("</table><br>\n");
		printf("共找到%d个符合条件的讨论区.\n", total);
	}
}
