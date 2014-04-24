#include "configbutton.h"


void configNew() {
	FILE *fp;
	char path[PATH_MAX];


	sprintf(path, "%s/.config-button/plugins-enabled", getenv("HOME"));

	if ((fp = fopen(path, "r"))) {
		fclose(fp);
		return;
	}

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
