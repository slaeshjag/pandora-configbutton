#include "../include/configbutton.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
	int		setspeed;
} INTERNAL;

int activate(void *internal) {
	char cpucmd[256];
	INTERNAL *ip = internal;

	if (ip->setspeed == 0)
		sprintf(cpucmd, "sudo -n /usr/pandora/scripts/op_cpuspeed.sh");
	else
		sprintf(cpucmd, "sudo -n /usr/pandora/scripts/op_cpuspeed.sh %i", ip->setspeed);

	system(cpucmd);
	return 0;
}


int getinfo(PLUGIN_INFO *info) {
	int i;
	struct PLUGIN_SUBMENU *sub;
	char cpuspeed[10];
	FILE *fp;
	char *nnn;
	INTERNAL *internal;
	
	if (info->submenu == NULL) {
		info->label = malloc(128);
		for (i = 3; i >= 0; i--) {
			sub = malloc(sizeof(struct PLUGIN_SUBMENU));
			sub->next = info->submenu;
			nnn = malloc(32);
			sprintf(nnn, "%i MHz", 500+i*100);
			sub->label = nnn;
			sub->icon_path = "NO icon";
			sub->visible = 1;
			internal = malloc(sizeof(INTERNAL));
			internal->setspeed = 500+i*100;
			sub->internal = internal;
			info->submenu = sub;
		}
		
		sub = malloc(sizeof(struct PLUGIN_SUBMENU));
		sub->next = info->submenu;
		sub->label = "Custom";
		sub->icon_path = "No icon";
		sub->visible = 1;
		internal = malloc(sizeof(INTERNAL));
		internal->setspeed = 0;
		sub->internal = internal;
		info->submenu = sub;
	}

	fp = fopen("/proc/pandora/cpu_mhz_max", "r");
	if (fp != NULL) {
		fscanf(fp, "%s", cpuspeed);
		fclose(fp);
	} else
		sprintf(cpuspeed, "N/A");
	sprintf((char *) info->label, "CPU Speed (Cur: %s MHz)", cpuspeed);
	info->icon_path = "hello.png";
	
	info->sort_hint = 0;

	return 0;
}
