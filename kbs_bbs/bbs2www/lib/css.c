#include <unistd.h>

char* bgcolor[]={
	"#000000",
	"#800000",
	"#008000",
	"#808000",
	"#000080",
	"#800080",
	"#008080",
	"#ffffff"};
char* fgcolor[]={
	"#000000",
	"#800000",
	"#008000",
	"#808000",
	"#000080",
	"#800080",
	"#008080",
	"#c0c0c0",
	"#808080",
	"#ff0000",
	"#00ff00",
	"#ffff00",
	"#0000ff",
	"#ff00ff",
	"#00ffff",
	"#ffffff"};
main()
{
int i,j;
for (i=0;i<8;i++)
	for (j=0;j<16;j++) {
		printf("FONT.f%d%02d {color: %s; background-color: %s }\n",i,j,fgcolor[j],bgcolor[i]);
	}
}
