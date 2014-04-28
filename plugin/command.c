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


struct {
	struct entry	*entry;
	int		entries;
} internal;


int activate(void *internal) {
	return 0;
}


int getinfo(PLUGIN_INFO *info) {
	if (!info->internal) {
		/* Init */
	}

	return 0;
}
