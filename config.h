#ifndef __CONFIG_H__
#define	__CONFIG_H__


struct config {
	char			**name;
	int			names;
};

void configLoad(struct configbutton *c);
int configShouldLoad(struct configbutton *c, const char *name);


#endif
