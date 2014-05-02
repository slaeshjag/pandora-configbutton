#include "../include/configbutton.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


const char plugin_name[] = "Execute command";
const char plugin_desc[] = "Adds a menu of user-defined shell commands";

#define	PRIMARY_ICON	"/usr/share/icons/pandora/exec_command.png"

struct entry {
	char		*command;
	char		*icon;
	char		*name;
};


struct internal {
	struct entry	*entry;
	int		entries;
};


int activate(void *internal) {
	return 0;
}


void pluginExecute(char *command) {
	system(command);
	return;
}


int getinfo(PLUGIN_INFO *info) {
	struct internal *in;
	int i;
	char path[PATH_MAX], buff_name[64], buff_icon[PATH_MAX], buff_command[2048];
	FILE *fp;
	struct PLUGIN_SUBMENU *sub;

	in = info->internal;
	if (!info->internal) {
		sprintf(path, "%s/.config-button/commands.presets", getenv("HOME"));
		if (!(fp = fopen(path, "r"))) {
			fprintf(stderr, "Unable to load commands\n");
			return -1;
		}
		

		in = malloc(sizeof(*in));
		in->entry = NULL;
		in->entries = 0;

		for (i = 0; !feof(fp); i++) {
			*buff_name = *buff_icon = *buff_command = 0;
			fscanf(fp, "%[^\t\n] %[^\t\n] %[^\t\n]\n", buff_name, buff_icon, buff_command);
			if (!(*buff_name))
				break;
			in->entry = realloc(in->entry, (i + 1) * sizeof(*in->entry));
			in->entries = i + 1;
			in->entry[i].command = strdup(buff_command);
			if (*buff_icon)
				in->entry[i].icon = strdup(buff_icon);
			else
				in->entry[i].icon = strdup(PRIMARY_ICON);
			in->entry[i].name = strdup(buff_name);
		}

		info->internal = in;

		info->label = "Execute command";
		info->icon_path = PRIMARY_ICON;
		info->sort_hint = 5;

		for (i = in->entries - 1; i >= 0; i--) {
			sub = info->submenu;
			info->submenu = malloc(sizeof(*info->submenu));
			info->submenu->next = sub;
			info->submenu->label = in->entry[i].name;
			info->submenu->internal = in->entry[i].command;
			info->submenu->icon_path = in->entry[i].icon;
			info->submenu->visible = 1;
			info->submenu->activate = pluginExecute;
		}
	}

	return 0;
}
