#include "./bbs.h"
#define TEMPLATE_DIR ".templ"
#define MAX_TEMPLATE 10
#define MAX_CONTENT 20
#define TMPL_BM_FLAG 0x1
#define MAX_CONTENT_LENGTH 555
#define TMPL_NOW_VERSION 1
#define TMPL_OLD_VERSION 0

struct s_template_old{
	char title[50];
	char title_prefix[20];
	int content_num;
	char filename[STRLEN];
	int flag;
	int version;
	char unused[16];
};

struct aa_template{
	struct s_template * tmpl;
	struct s_template_old * tmpl_old;
	struct s_content * cont;
} * ptemplate = NULL;

int template_num = 0;
int t_now = 0;

main(){

	FILE *fp;
	struct boardheader bh;

	chdir(BBSHOME);

	if((fp=fopen(BOARDS,"r"))==NULL){
		printf("cannot open .BOARDS\n");
		exit(0);
	}
	while(fread(&bh, sizeof(bh), 1, fp)){
		if( bh.filename[0] == '\0' ) continue;
		printf("change: %s\n",bh.filename);
		if(tmpl_init(bh.filename)>0){
			tmpl_save(bh.filename);
		}
		tmpl_free();
		ptemplate = NULL;
		template_num = 0;
		t_now = 0;
	}
	fclose(fp);
}


int tmpl_init(char *boardname){
		/***********
		 * mode 0: 用户查看
		 * mode 1: 斑竹管理
		 ************/

	int fd,i;
	char tmpldir[STRLEN];
	struct s_template_old tmpl;
	struct s_template tmpl_new;
	struct s_content * cont;

    setbfile(tmpldir, boardname, TEMPLATE_DIR);

	if( ptemplate == NULL ){
		ptemplate = (struct aa_template *) malloc( sizeof( struct aa_template ) * MAX_TEMPLATE );
		if( ptemplate == NULL )
			return -1;
	}
	bzero( ptemplate, sizeof( struct aa_template ) * MAX_TEMPLATE );
	template_num = 0;

	if( (fd = open( tmpldir, O_RDONLY ) ) == -1 ){
		return 0;
	}
	while( read(fd, &tmpl, sizeof( struct s_template_old )) == sizeof(struct s_template_old) ){
		if( tmpl.version >= TMPL_NOW_VERSION ){
			printf("已处理过\n");
			close(fd);
			return -1;
		}
		else
			printf("ok\n");
		cont = (struct s_content *) malloc( sizeof( struct s_content ) * tmpl.content_num );
		if( cont == NULL )
			break;
		bzero(cont, sizeof(struct s_content) * tmpl.content_num );
		if(read(fd, cont, sizeof(struct s_content)*tmpl.content_num) != sizeof(struct s_content)*tmpl.content_num)
		 	continue;
		ptemplate[template_num].tmpl_old = (struct s_template_old *)malloc(sizeof(struct s_template_old));
		ptemplate[template_num].tmpl = (struct s_template *)malloc(sizeof(struct s_template));
		if( ptemplate[template_num].tmpl_old == NULL ){
			free(cont);
			break;
		}
		bzero( ptemplate[template_num].tmpl_old , sizeof(struct s_template_old) );
		bzero( ptemplate[template_num].tmpl , sizeof(struct s_template) );
		bzero( &tmpl_new , sizeof(struct s_template) );
		memcpy( ptemplate[template_num].tmpl_old, &tmpl, sizeof(struct s_template_old) );
		memcpy( &tmpl_new, &tmpl, sizeof(struct s_template_old) );
		tmpl_new.version = TMPL_NOW_VERSION;
		memcpy( ptemplate[template_num].tmpl, &tmpl_new, sizeof(struct s_template) );
		ptemplate[template_num].cont = cont;
		template_num ++;
		if( template_num >= MAX_TEMPLATE )
			break;
	}
	close(fd);

	return template_num;
}

int tmpl_free(){

	int i;

	for(i=0; i<template_num; i++){
		free(ptemplate[i].tmpl);
		free(ptemplate[i].cont);
		free(ptemplate[i].tmpl_old);
	}
	free(ptemplate);
	ptemplate = NULL;
}

int tmpl_save(char *boardname){

	int i;
	FILE *fp;
	char tmpldir[STRLEN];

    setbfile(tmpldir, boardname, TEMPLATE_DIR);
	if( (fp = fopen( tmpldir, "w") ) == NULL ){
		return -1;
	}
	for(i=0; i<template_num; i++){
		if(ptemplate[i].tmpl == NULL)
			continue;
		fwrite( ptemplate[i].tmpl, sizeof(struct s_template), 1, fp );
		fwrite( ptemplate[i].cont, sizeof(struct s_content), ptemplate[i].tmpl->content_num, fp);
	}
	fclose(fp);

	return 0;
}
