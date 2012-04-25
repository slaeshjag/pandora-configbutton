#include "../include/configbutton.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
	char			modbuff[65536];			// 64 kB should be enough for everyone!
	unsigned int		action;
} INTERNAL;


int activate(void *internal) {
	INTERNAL *ip = internal;

	if (ip->action == 0) {
		system("/usr/pandora/scripts/op_bluetooth.sh");
	} else {
		system("/usr/pandora/scripts/op_bluetooth.sh");
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
	
	if ((fp = popen("hcitool dev", "r")) == NULL)
		return -1;
	internal->modbuff[fread(internal->modbuff, 1, 65536, fp)] = 0;
	if (strstr(internal->modbuff, "hci") != NULL) {
		internal->action = 0;
		info->label = "Disable Bluetooth";
		info->icon_path = "bluetooth_disable.png";
	} else {
		info->label = "Enable Bluetooth";
		info->icon_path = "bluetooth_enable.png";
	}

	info->sort_hint = 1;

	return 0;
}
