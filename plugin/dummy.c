#include "../include/configbutton.h"
#include <stdio.h>
#include <stdlib.h>



int activate(void *internal) {
	fprintf(stderr, "ACTIVATE!\n");

	return 0;
}


int getinfo(PLUGIN_INFO *info) {
	int i;
	struct PLUGIN_SUBMENU *sub;
	if (info->submenu == NULL) {
		for (i = 0; i < 4; i++) {
			sub = malloc(sizeof(struct PLUGIN_SUBMENU));
			sub->next = info->submenu;
			sub->label = "Hello, submenu";
			sub->icon_path = "NO icon";
			sub->visible = 1;
			info->submenu = sub;
		}
	}

	info->label = "Submenu";
	info->icon_path = "hello.png";
	
	info->sort_hint = 0;

	return 0;
}
