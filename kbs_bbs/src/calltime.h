#ifndef _CALLTIME_H
#define _CALLTIME_H

#define CLOCK_TYPE_NORMAL 1
#define CLOCK_TYPE_DAY 2
#define CLOCK_TYPE_WEEK 3
#define CLOCK_TYPE_MONTH 4
#define CLOCK_TYPE_YEAR 5
#define CLOCK_TYPE_IMP_NOT 6
#define CLOCK_TYPE_IMP_HAD 7
#define CLOCK_TYPE_LOGIN 8
#define CLOCK_TYPE_LOGIN_HAD 9

#define MAX_CLOCK_DEF 100

struct clock_struct
{
	int type;
	time_t clock_time;
	char memo[40];
};

#endif
