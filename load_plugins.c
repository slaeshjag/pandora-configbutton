#include "configbutton.h"



int updatePlugins(CONFIGBUTTON *c) {
	struct PLUGIN_ENTRY *plugin;

	plugin = c->entry;

	while (plugin != NULL) {
		(plugin->getinfo)(&plugin->info);
		plugin = plugin->next;
	}
	
	return 0;
}


int openPlugin(CONFIGBUTTON *c, const char *fname) {
	void *libhandle;
	char fname_dot[256];
	struct PLUGIN_ENTRY *plugin;

	sprintf(fname_dot, "./%s", fname);

	if ((libhandle = dlopen(fname_dot, RTLD_NOW | RTLD_GLOBAL)) == NULL) {
		return -1;
	}

	if ((plugin = malloc(sizeof(struct PLUGIN_ENTRY))) == NULL) {
		dlclose(libhandle);
		return -1;
	}

	plugin->library = libhandle;
	if ((plugin->getinfo = dlsym(libhandle, "getinfo")) == NULL) {
		fprintf(stderr, "Plugin %s does not have the required symbol 'getinfo'\n", fname_dot);
		dlclose(plugin->library);
		free(plugin);
		return -1;
	}

	if ((plugin->activate = dlsym(libhandle, "activate")) == NULL) {
		fprintf(stderr, "Plugin %s does not have the required symbol 'activate'\n", fname_dot);
		dlclose(plugin->library);
		free(plugin);
		return -1;
	}

	plugin->free_name = 0;
	if (!(plugin->name = dlsym(libhandle, "plugin_name")))
		plugin->name = strdup(fname), plugin->free_name = 1;

	if (!configShouldLoad((struct configbutton *) c, plugin->name)) {
		fprintf(stderr, "Plugin %s is not in load list\n", plugin->name);
		dlclose(plugin->library);
		if (plugin->free_name) free((void *) plugin->name);
		free(plugin);
		return -1;
	}

	fprintf(stderr, "Found plugin '%s'\n", plugin->name);

	plugin->item = NULL;
	plugin->info.internal = NULL;
	plugin->info.submenu = NULL;
	plugin->next = c->entry;
	if (c->entry == NULL);
	else c->entry->prev = plugin;
	c->entry = plugin;
	(plugin->getinfo)(&plugin->info);
	c->plugins++;

	return 0;
}


int loadPlugins(CONFIGBUTTON *c) {
	DIR *dir;
	struct dirent *file;
	PLUGIN_STRUCT tmp;
	struct PLUGIN_ENTRY *entry;
	int i, j;

	dir = opendir(".");

	do {
		file = readdir(dir);
		if (file == NULL)
			break;
		openPlugin(c, file->d_name);
	} while (1);

	closedir(dir);

	/* This is a bit ugly, but I was unable to get sorting with linked lists working */

	if (c->plugin != NULL) free(c->plugin);
	if ((c->plugin = malloc(sizeof(PLUGIN_STRUCT)*c->plugins)) == NULL) {
		c->plugins = 0;
		return -1;
	}

	entry = c->entry;
	for (i = 0; i < c->plugins; i++) {
		c->plugin[i].library = entry->library;
		c->plugin[i].getinfo = entry->getinfo;
		c->plugin[i].activate = entry->activate;
		c->plugin[i].item = entry->item;
		c->plugin[i].info = &entry->info;
		entry = entry->next;
	}

	for (i = 0; i < c->plugins; i++)
		for (j = i; j > 0 && c->plugin[j].info->sort_pos < c->plugin[j-1].info->sort_pos; j--) {
			tmp = c->plugin[j];
			c->plugin[j] = c->plugin[j-1];
			c->plugin[j-1] = tmp;
		}
			
	return 0;
}


int initPlugins(CONFIGBUTTON *c) {
	char *home;
	c->entry = NULL;
	c->plugin = NULL;
	c->plugins = 0;

	loadPlugins(c);
	home = getenv("HOME");

	chdir(home);
	if (chdir(".config-button") == 0)
		loadPlugins(c);
	
	chdir("/usr/share/configbutton");
	loadPlugins(c);

	return 0;
}
