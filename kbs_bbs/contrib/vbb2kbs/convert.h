#define HOST "59.66.118.60"
#define PORT 3306
#define USER "software"
#define PASS "sex"
#define DB "free"
#define ATTACHPATH "/opt/a"
#define HIDEIP
const int forumid[] = {33,0};
const char* boardname[] = {"ConvertTest", NULL};

char* vbbuser2bbsid(char* user, int i){
	static char buf[128];
	switch (i) {
		case 99999: return "FreeWizard";
	}
	if (id_invalid(user)||(strlen(user)>11)) {
		sprintf(buf, "vbuser%d.", i);
	} else {
		sprintf(buf, "%s.", user);
	}
	return buf;
}
