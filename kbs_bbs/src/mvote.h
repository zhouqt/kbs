struct vote_class {
        int qid;
        int type;
        char title[80];
        int constrain ;
        int IsAns ;
} ;
#define Vote_YesNo      (1)
#define Vote_Single     (2)
#define Vote_Multi      (3)
#define Vote_Value      (4)
#define Vote_Asking     (5)
