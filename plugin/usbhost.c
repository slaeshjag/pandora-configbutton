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
		system("gksudo rmmod ehci-hcd");
	} else {
		system("gksudo modprobe ehci-hcd");
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
	if (strstr(internal->modbuff, "ehci_hcd") != NULL) {
		internal->action = 0;
		info->label = "Disable USB-host";
		info->icon_path = "usbhost_disable.png";
	} else {
		internal->action = 1;
		info->label = "Enable USB-host";
		info->icon_path = "usbhost_enable.png";
	}

	info->sort_hint = 2;

	return 0;
}
