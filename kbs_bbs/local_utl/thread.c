#undef BBSMAIN
#include "bbs.h"

char tname[STRLEN];
char fname[STRLEN];

struct postnode
{
    int num;
    struct postnode *next;
};

struct titlenode
{
    char *title;
    struct titlenode *next;
    struct postnode *post;
};

struct titlenode toptitle;

int
thread(post,num)
struct fileheader *post;
int num;
{
    struct titlenode *tmp;
    char *ntitle;

    RemoveAppendedSpace(post->title); /* Leeward 98.02.13 */

    /*        printf("%d ",num);*/
    tmp=&toptitle;
    if(post->title[0]=='R'&&post->title[1]=='e'&&post->title[2]==':')
        ntitle=post->title+4;
    else
        ntitle=post->title;
    while(1)
    {
        if(tmp->next==NULL)
        {
            struct titlenode *titletmp;
            titletmp=(struct titlenode *)malloc(sizeof(struct titlenode));
            titletmp->title=malloc(sizeof(char)*(strlen(ntitle)+1));
            strcpy(titletmp->title,ntitle);
            titletmp->post=NULL;
            titletmp->next=NULL;
            tmp->next=titletmp;
        }
        if(!strcmp(tmp->next->title,ntitle))
        {
            struct postnode *tmppost,*first=tmp->next->post;
            if(first==NULL)
            {
                tmppost=(struct postnode *)malloc(sizeof(struct postnode));
                tmppost->num=num;
                tmppost->next=NULL;
                tmp->next->post=tmppost;
                return;
            }
            while(1)
            {
                if(first->next==NULL)
                {
                    tmppost=(struct postnode *)malloc(sizeof(struct postnode));
                    tmppost->num=num;
                    tmppost->next=NULL;
                    first->next=tmppost;
                    return;
                }
                first=first->next;
            }
        }else
        {
            tmp=tmp->next;
        }
    }
}

int
visit_all()
{
    struct titlenode *tmp;
    struct fileheader post;
    int i=0;

    tmp=toptitle.next;

    while(tmp)
    {
        struct postnode *tmppost;

        i++;
        tmppost=tmp->post;
        while(tmppost)
        {
            get_record(fname,&post,sizeof(post),tmppost->num);
            if( !strncmp(post.title,"Re: ",4) || !strncmp(post.title,"RE: ",4) )
            {
                if(tmppost->next)
                    sprintf(post.title,"©À %s",post.title+4);
                else
                    sprintf(post.title,"©¸ %s",post.title+4);
            }
            append_record(tname,&post,sizeof(post));
            tmppost=tmppost->next;
        }
        tmp=tmp->next;
    }

}

int
main(argc, argv)
char *argv[];
int argc;
{
    FILE *tf;
    int i=0;
    struct fileheader post;
    char dname[STRLEN];
    char buf[256];
    struct stat st1,st2;

    sprintf(dname,"boards/%s/%s",argv[1],DOT_DIR);
    sprintf(fname,"boards/%s/%s2",argv[1],DOT_DIR);
    sprintf(tname,"boards/%s/%s",argv[1],THREAD_DIR);

    if(stat(dname,&st1)==-1)
        return;
    if(stat(tname,&st2)!=-1)
    {
        if(st2.st_mtime>=st1.st_mtime)
            return;
    }

    unlink(tname);
    sprintf(buf,"cp %s %s",dname,fname);
    system(buf);

    if((tf=fopen(fname,"rb"))==NULL)
    {
        printf(".DIR cant open...");
        return ;
    }
    toptitle.next=NULL;
    toptitle.post=NULL;
    while(1)
    {
        i++;
        if(fread(&post,sizeof(post),1,tf)<=0) break;
        thread(&post,i);
    }
    visit_all();
    fclose(tf);
    unlink(fname);
}
