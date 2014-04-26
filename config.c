#include "configbutton.h"
#include <sys/stat.h>
#include <sys/types.h>

void configNew() {
	FILE *fp;
	char path[PATH_MAX];
	char dir[PATH_MAX];


	sprintf(path, "%s/.config-button/plugins-enabled", getenv("HOME"));

	if ((fp = fopen(path, "r"))) {
		fclose(fp);
		return;
	}

	sprintf(dir, "%s/.config-button", getenv("HOME"));
	mkdir(dir, 0755);
	if (!(fp = fopen(path, "w"))) {
		fprintf(stderr, "Unable to create %s\n", path);
		return;
	}

	fprintf(fp, "Toggle WIFI\n");
	fprintf(fp, "TV-Out settings\n");
	fprintf(fp, "Toggle Bluetooth™\n");
	fprintf(fp, "Toggle USB-host\n");
	fprintf(fp, "USB Mass storage\n");

	fclose(fp);
}


void configLoad(struct configbutton *c) {
	char *buff, *newbuff, path[4096];
	size_t size;
	FILE *fp;
	int i;

	configNew();
	sprintf(path, "%s/.config-button/plugins-enabled", getenv("HOME"));
	fp = fopen(path, "r");
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buff = malloc(size + 1);
	fread(buff, size, 1, fp);
	buff[size] = 0;
	fclose(fp);
	newbuff = buff;

	for (i = 0; (newbuff = strchr(newbuff, '\n')); newbuff++, i++);

	c->config.name = malloc(sizeof(char *) * i);
	c->config.names = i;
	newbuff = buff;

	for (i = 0; i < c->config.names; i++) {
		c->config.name[i] = newbuff;
		newbuff = strchr(newbuff, '\n');
		*newbuff = 0;
		newbuff++;
		fprintf(stderr, "Should load %s\n", c->config.name[i]);
	}
	
	return;
}


int configShouldLoad(struct configbutton *c, const char *name) {
	int i;

	if (!name)
		return 0;
	
	for (i = 0; i < c->config.names; i++)
		if (!strcmp(c->config.name[i], name))
			return 1;

	return 0;
}


void configInitFound() {
	cb->info.info = NULL;
	cb->info.infos = 0;
	
	return;
}


void configAddFound(const char *name, const char *desc, int loaded) {
	int id;
	id = cb->info.infos++;
	cb->info.info = realloc(cb->info.info, sizeof(*cb->info.info) * cb->info.infos);
	
	cb->info.info[id].name = strdup(name);
	if (desc)
		cb->info.info[id].description = strdup(desc);
	else
		cb->info.info[id].description = "No description available";
	cb->info.info[id].loaded = loaded;

	return;
}


char *configFindFound(const char *name) {
	int i;
	for (i = 0; i < cb->info.infos; i++) {
		if (!strcmp(cb->info.info[i].name, name))
			return cb->info.info[i].description;
	}

	return "Invalid plugin";
}	


void configSetFoundLoaded(const char *name, int loaded) {
	int i;
	for (i = 0; i < cb->info.infos; i++) {
		if (!strcmp(cb->info.info[i].name, name))
			cb->info.info[i].loaded = loaded;
	}
}


void configSaveLoaded() {
	char path[PATH_MAX];
	int i;
	FILE *fp;

	sprintf(path, "%s/.config-button/plugins-enabled", getenv("HOME"));
	if (!(fp = fopen(path, "w")))
		return;
	for (i = 0; i < cb->info.infos; i++) {
		if (cb->info.info[i].loaded)
			fprintf(fp, "%s\n", cb->info.info[i].name);
	}

	fclose(fp);
	return;
}
