#include "../include/configbutton.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char plugin_name[] = "Toggle WIFI";

typedef struct {
	char			modbuff[65536];			// 64 kB should be enough for everyone!
	unsigned int		action;
} INTERNAL;


int activate(void *internal) {
	INTERNAL *ip = internal;

	if (ip->action == 0) {
		system("/usr/pandora/scripts/pnd_run.sh -p '/usr/pandora/apps/op_wifi.pnd' -e 'op_wifi.sh' -b 'op_wifi' &");
	} else {
		system("/usr/pandora/scripts/pnd_run.sh -p '/usr/pandora/apps/op_wifi.pnd' -e 'op_wifi.sh' -b 'op_wifi' &");
	}

	return 0;
}


int getinfo(PLUGIN_INFO *info) {
	FILE *fp;
	INTERNAL *internal = info->internal;

	if (info->internal == NULL) {
		internal = malloc(sizeof(INTERNAL));
		internal->action = 0;
		info->internal = internal;
	}
	
	if ((fp = fopen("/proc/modules", "r")) == NULL)
		return -1;
	internal->modbuff[fread(internal->modbuff, 1, 65536, fp)] = 0;
	if (strstr(internal->modbuff, "wl1251") != NULL) {
		internal->action = 0;
		info->label = "Disable WiFi";
		info->icon_path = "/usr/share/icons/pandora/wifi.png";
	} else {
		internal->action = 1;
		info->label = "Enable WiFi";
		info->icon_path = "/usr/share/icons/pandora/wifi.png";
	}

	info->sort_hint = 40;

	return 0;
}
