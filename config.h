#ifndef __CONFIG_H__
#define	__CONFIG_H__

struct config_info_entry {
	char			*name;
	char			*description;
	int			loaded;
};

struct config_info {
	struct config_info_entry *info;
	int			infos;
};

struct config {
	char			**name;
	int			names;
};

void configLoad(struct configbutton *c);
int configShouldLoad(struct configbutton *c, const char *name);

void configInitFound();
void configAddFound(const char *name, const char *desc, int loaded);
char *configFindFound(const char *name);
void configSetFoundLoaded(const char *name, int loaded);
void configSaveLoaded();


#endif
