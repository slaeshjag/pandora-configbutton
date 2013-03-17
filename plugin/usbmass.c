#include "../include/configbutton.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
	int	action;
	char	path[512];
} INTERNAL;


int activate(void *internal) {
	INTERNAL *ip = internal;
	char cmdline[1024];

	if( ip->action != 0 )
	{
		sprintf(cmdline, "sudo -n /usr/pandora/scripts/op_storage.sh %s", ip->path);
		system(cmdline);
	}
	return 0;
}


int getinfo(PLUGIN_INFO *info) {
	FILE *fp;
	INTERNAL *internal = info->internal;
	struct PLUGIN_SUBMENU *sub;
	char opPath[512];
	char *pathStr;

	if (info->internal == NULL) {
		internal = malloc(sizeof(INTERNAL));
		internal->action = 0;
		info->internal = internal;
	}
	
	internal->action = 0;
	info->label = "SD USB Mass Storage";
	info->icon_path = "/usr/share/icons/pandora/usb.png";
	info->sort_hint = 32;

	while( info->submenu != NULL )
	{
		sub = info->submenu;
		info->submenu = sub->next;
		free(sub->label);
		free(sub->internal);
		free(sub);
	}

	fp = popen("sudo -n /usr/pandora/scripts/op_storage.sh list", "r");
	if (fp != NULL) {
		while( fscanf(fp, "%s", opPath) > 0 )
		{
			sub = malloc(sizeof(struct PLUGIN_SUBMENU));
			internal = malloc(sizeof(INTERNAL));
			sub->next = info->submenu;
			pathStr = malloc(512);
			strcpy(pathStr, opPath);
			sub->label = pathStr;
			sub->icon_path = "/usr/share/icons/pandora/usb.png";
			sub->visible = 1;
			strcpy(internal->path, opPath);
			internal->action = 1;
			sub->internal = internal;
			info->submenu = sub;
		}
		fclose(fp);
	}		

	return 0;
}
