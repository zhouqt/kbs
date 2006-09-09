/* etnlegend, 2006.09.10, 导出用户身份设定数据... */
#include "bbs.h"

#define TIDMAX 255

static const struct userec *buf[MAXUSERS];
static int count;

static int dut_create_buffer(struct userec *user,void *varg){
    if(*(user->userid)&&user->title)
        buf[count++]=user;
    return 0;
}

static int dut_sort_routine(const void *p,const void *q){
    return (((*(const struct userec**)p)->title)-((*(const struct userec**)q)->title));
}

int main(void){
    int idx;
    const char *des;
    unsigned char prev;
    if(chdir(BBSHOME)==-1)
        return -1;
    resolve_ucache();
    apply_users(dut_create_buffer,NULL);
    qsort(buf,count,sizeof(const struct userec*),dut_sort_routine);
    fprintf(stdout,"%s\n","---- Current User Title Configuration ----");
    for(prev=0,idx=0;idx<count;idx++){
        if(prev!=buf[idx]->title){
            while(++prev!=buf[idx]->title)
                if(*(des=get_user_title(prev)))
                    fprintf(stdout,"\n[%3d] [%s]\n",prev,des);
            if(!*(des=get_user_title(prev)))
                des="(NULL)";
            fprintf(stdout,"\n[%3d] [%s]\n",prev,des);
        }
        fprintf(stdout,"    %s\n",buf[idx]->userid);
    }
    while(++prev!=TIDMAX)
        if(*(des=get_user_title(prev)))
            fprintf(stdout,"\n[%3d] [%s]\n",prev,des);
    fprintf(stdout,"\n%s\n","------------------------------------------");
    return 0;
}

