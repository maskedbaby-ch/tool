#ifndef _NETSTATE_H_
#define _NETSTATE_H_

enum {
	OPT_START,
	OPT_STOP,
	OPT_RELOAD,
	OPT_RESTART
};

enum {
	STAT_UP,
	STAT_DOWN
};

#define MAX_SCRIPT_NUM 32

struct opt_script_arr {
	char name[64];
	int stat;
	int opt;	
	int (*run)(struct opt_script_arr *ptr);
}opt_arr[MAX_SCRIPT_NUM];

int opt_run_script(struct opt_script_arr *ptr)
{
	char cmd[128] = {0};
	memset(cmd, 0, sizeof(cmd));
	switch(ptr->opt) {
		case OPT_START: break;
		case OPT_STOP: break;
		case OPT_RELOAD: {
			sprintf(cmd, "%s reload", ptr->name);
			break;
		}
		case OPT_RESTART: {
			sprintf(cmd, "%s restart", ptr->name);
			break;
		}
		default: break;
	}
	system(cmd);
	return 0;
}

int opt_register_script(char *script, int stat, int opt)
{
	for(int i=0; i<MAX_SCRIPT_NUM; i++) {
		if(strlen(opt_arr[i].name) == 0){
			memcpy(opt_arr[i].name, script, strlen(script));
			opt_arr[i].stat = stat;
			opt_arr[i].opt = opt;
			opt_arr[i].run = opt_run_script;
			break;
		}
	}
	return 0;
}

int run_script(int stat)
{
	for(int i=0; i<MAX_SCRIPT_NUM; i++) {
		if (stat == opt_arr[i].stat) {
			opt_run_script(&opt_arr[i]);
		}
	}
}

#endif
