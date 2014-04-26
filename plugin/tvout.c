#include "../include/configbutton.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
	int		tvout_mode;
} INTERNAL;


const char plugin_name[] = "TV-Out settings";
const char plugin_desc[] = "Adds a menu for selecting TV-out output mode";
const char *label[32] = { "PAL, Main Layer", "PAL, HW Scaler", "NTSC, Main Layer", "NTSC, HW Scaler", "Disable TV-out", "Configure TV-out" };


int activate(void *internal) {
	INTERNAL *ip = internal;

	switch (ip->tvout_mode) {
		case 0:			// PAL, Layer 0
			system("sudo -n /usr/pandora/scripts/op_tvout.sh -t pal -l 0");
			break;
		case 1:			// PAL, Layer 1
			system("sudo -n /usr/pandora/scripts/op_tvout.sh -t pal -l 1");
			break;
		case 2:			// NTSC, Layer 0
			system("sudo -n /usr/pandora/scripts/op_tvout.sh -t ntsc -l 0");
			break;
		case 3:			// NTSC, Layer 1
			system("sudo -n /usr/pandora/scripts/op_tvout.sh -t ntsc -l 1");
			break;
		case 4:			// Disable
			system("sudo -n /usr/pandora/scripts/op_tvout.sh -d");
			break;
		default:		// Configure
			system("/usr/pandora/scripts/TVoutConfig.py &");
			break;
	}
	
	return 0;
}


int getinfo(PLUGIN_INFO *info) {
	int i;
	struct PLUGIN_SUBMENU *sub;
	INTERNAL *internal;
	
	if (info->submenu == NULL) {
		info->label = malloc(128);
		for (i = 5; i >= 0; i--) {
			sub = malloc(sizeof(struct PLUGIN_SUBMENU));
			sub->next = info->submenu;
			sub->label = label[i];
			sub->icon_path = "/usr/share/icons/pandora/tvout.png";
			sub->visible = 1;
			internal = malloc(sizeof(INTERNAL));
			internal->tvout_mode = i;
			sub->internal = internal;
			info->submenu = sub;
		}
	}

	info->label = "TV-out";
	info->icon_path = "/usr/share/icons/pandora/tvout.png";
	
	info->sort_hint = 10;

	return 0;
}
