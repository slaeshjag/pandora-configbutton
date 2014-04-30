#include "../include/configbutton.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const char plugin_name[] = "Execute command";
const char plugin_desc[] = "Adds a menu of user-defined shell commands";

struct entry {
	char		*command;
	char		*name;
};


struct internal {
	struct entry	*entry;
	int		entries;
};


int activate(void *internal) {
	return 0;
}


int getinfo(PLUGIN_INFO *info) {
	struct internal *i;
	int i;
	char path[PATH_MAX];
	FILE *fp;

	i = info->internal;
	if (!info->internal) {
		sprintf(path, "%s/.config-button/commands.presets", getenv("HOME"));
		if (!(fp = fopen(path, "r"))) {
			fprintf(stderr, "Unable to load commands\n");
			return -1;
		}
		

		/* Figure out how many commands there are */
		
		i = malloc(sizeof(*i));
		/* Init */
	}

	return 0;
}
