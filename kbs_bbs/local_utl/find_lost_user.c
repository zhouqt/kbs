#include "bbs.h"

int main(argc, argv)
{ 
    DIR *dp;
    struct dirent *dirp; 
    DIR *sdp;
    struct dirent *sdirp;
    struct userec *lookupuser;
    char opername[60];
    
    chdir(BBSHOME);
    resolve_ucache();
    if((dp=opendir("home"))==NULL) 
        printf("cann't open home directory!\n");
    else
    {
        while ((dirp=readdir(dp))!=NULL) 
        {
            chdir("home");
            if((sdp=opendir(dirp->d_name))==NULL) 
                printf("cann't open home/%s directory!\n",dirp->d_name); 
            else
            {
                if(strcmp(dirp->d_name,".")!=0 && strcmp(dirp->d_name,"..")!=0)
                {
                    while((sdirp=readdir(sdp)) != NULL )
                    {
                        if( strcmp(sdirp->d_name,".")!=0 && strcmp(sdirp->d_name,"..")!=0)
                        {
                            if( getuser(sdirp->d_name, &lookupuser) == 0 )
                            {
                                sprintf(opername,"mv %s/%s/ bak/", dirp->d_name, sdirp->d_name );
                                system(opername);
                                
                                sprintf(opername,"mv ../mail/%s/%s/ ../mail/bak/", dirp->d_name, sdirp->d_name );
                                system(opername);
                                printf("%s\n" , sdirp->d_name);    
                            }
                        }
                    }
                }
                closedir(sdp);
            }
        }
        closedir(dp); 
    }
    return 0;
} 
