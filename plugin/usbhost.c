#include "../include/configbutton.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const char plugin_name[] = "Toggle USB-host";
const char plugin_desc[] = "Adds a menu for enabling/disabling the USB host port";

typedef struct {
	char			modbuff[65536];			// 64 kB should be enough for everyone!
	unsigned int		action;
} INTERNAL;


int activate(void *internal) {
	INTERNAL *ip = internal;

	if (ip->action == 0) {
		system("sudo /usr/pandora/scripts/op_usbhost.sh");
//		system("gksudo rmmod ehci-hcd");
	} else {
		system("sudo /usr/pandora/scripts/op_usbhost.sh");
//		system("gksudo modprobe ehci-hcd");
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
		info->icon_path = "/usr/share/icons/pandora/usb.png";
	} else {
		internal->action = 1;
		info->label = "Enable USB-host";
		info->icon_path = "/usr/share/icons/pandora/usb.png";
	}

	info->sort_hint = 30;

	return 0;
}
