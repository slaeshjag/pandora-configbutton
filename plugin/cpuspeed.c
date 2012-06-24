#include "../include/configbutton.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	int		setspeed;
	char		confbuff[1024];
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


int getMinCPU(INTERNAL *internal) {
	int number;
	char *mhz;

	if ((mhz = strstr(internal->confbuff, "min:")) == NULL) {
		fprintf(stderr, "Error: Unable to find MIN CPU-speed setting\n");
		return 600;
	}

	mhz += 4;
	sscanf(mhz, "%i", &number);

	return (number < 600) ? 600 : number;
}


int getMaxCPU(INTERNAL *internal) {
	FILE *fp;
	int number;
	char *mhz;

	if ((fp = fopen("/etc/pandora/conf/cpu.conf", "r")) == NULL) {
		fprintf(stderr, "Error: Unable to load /etc/pandora/conf/cpu.conf! Assuming 800 MHz...");
		return 800;
	}

	internal->confbuff[fread(internal->confbuff, 1, 1024, fp)] = 0;
	fclose(fp);

	if ((mhz = strstr(internal->confbuff, "max:")) == NULL) {
		fprintf(stderr, "Unable to find MAX CPU-speed setting\n");
		return 800;
	}
	mhz += 4;
	sscanf(mhz, "%i", &number);

	return number;
}


int getStep(int mhz, int min) {
	int i, j;

	for (i = 1, j = mhz; j > 5; i++) 
		j = (mhz - min + i * 25) / (i*25);
	return (i-1) * 25;
}


int getinfo(PLUGIN_INFO *info) {
	int i, step, max, min, loops;
	struct PLUGIN_SUBMENU *sub;
	char cpuspeed[10];
	FILE *fp;
	char *nnn;
	INTERNAL *internal;
	
	if (info->submenu == NULL) {
		internal = malloc(sizeof(INTERNAL));
		info->label = malloc(128);
		
		max = getMaxCPU(internal);
		min = getMinCPU(internal);
		step = getStep(max, min);
		loops = (max - min) / step;
		
		for (i = loops; i >= 0; i--) {
			sub = malloc(sizeof(struct PLUGIN_SUBMENU));
			sub->next = info->submenu;
			nnn = malloc(32);
			sprintf(nnn, "%i MHz", min + i * step);
			sub->label = nnn;
			sub->icon_path = "/usr/share/icons/pandora/cpu.png";
			sub->visible = 1;
			internal = malloc(sizeof(INTERNAL));
			internal->setspeed = min + i * step;
			sub->internal = internal;
			info->submenu = sub;
		}
		
		sub = malloc(sizeof(struct PLUGIN_SUBMENU));
		internal = malloc(sizeof(INTERNAL));
		sub->next = info->submenu;
		sub->label = "Custom";
		sub->icon_path = "/usr/share/icons/pandora/cpu.png";
		sub->visible = 1;
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
	info->icon_path = "/usr/share/icons/pandora/cpu.png";
	
	info->sort_hint = 0;

	return 0;
}
