/* 
 *	Convert the dumped txt for weather broadcast to
 *	formatted text
 *
 * 	Date: Apr 2003 By Bigman
*/


#include <stdio.h>
#define SIZE 4096

main(argc,argv)
	int argc;
	char *argv[];
{
	FILE *fp1;

	int i,j,m,p,q;

	char in_buf[SIZE];
	char buf[SIZE];

	if (argc!=2) {
		printf("Weather_sort filename\n");
		exit(0);
	}

	if ((fp1= fopen(argv[1],"r")) == NULL)
	{
		printf("can't open origalname\n");

		exit(0);
	}

	i=0;j=0;

	while (fgets(in_buf,SIZE,fp1)!=NULL)
	{
		if (i==93) {
			printf("\n %s \n\n",in_buf);
			printf("          天气现象    温 度（C）   风向 风力\n");
			printf("   城 市  夜间 白天   最高  最低   夜间 白天\n\n");
		}

		if ((i>96)&&(i<165))
		{
			j++;

			if (j==1) {
				strcpy(buf,in_buf);
				buf[strlen(buf)-1]=0;
			}
			if (i==120) strcpy(buf,"   拉萨");

			if (j==2)
			{
				printf ("%s %s\n",buf,in_buf);
				j=0;

			}


		}

		i++;
		memset(in_buf,0,SIZE);
	}

	printf("\n\n本服务由 http://www.nma.gov.cn提供\n\n\n");

	fclose(fp1);

											        exit(0);


}

 
