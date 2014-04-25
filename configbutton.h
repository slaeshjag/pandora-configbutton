#ifndef __CONFIGBUTTON_H__
#define	__CONFIGBUTTON_H__


#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <signal.h>

#include <sys/types.h>

struct configbutton;
#include "config.h"


typedef struct {
	const char		*label;
	const char		*icon_path;
	void			*internal;
	struct PLUGIN_SUB_ENTRY	*submenu;
	int			sort_pos;
} PLUGIN_INFO;


struct PLUGIN_SUB_ENTRY {
	GtkWidget		*item;
	const char		*label;
	void			*internal;
	const char		*icon_path;
	int			visible;
	struct PLUGIN_SUB_ENTRY	*next;
	int			(*activate)(void *internal);
};


typedef struct {
	void			*library;
	int			(*getinfo)(PLUGIN_INFO *info);
	int			(*activate)(void *internal);
	GtkWidget		*item;
	PLUGIN_INFO		*info;
} PLUGIN_STRUCT;


struct PLUGIN_ENTRY {
	const char		*name;
	int			free_name;
	void			*library;
	int			(*getinfo)(PLUGIN_INFO *info);
	int			(*activate)(void *internal);
	GtkWidget		*item;
	PLUGIN_INFO		info;
	struct PLUGIN_ENTRY	*next;
	struct PLUGIN_ENTRY	*prev;
};


typedef struct configbutton{
	GtkStatusIcon		*icon;
	GtkWidget		*menu;
	
	struct	PLUGIN_ENTRY	*entry;
	PLUGIN_STRUCT		*plugin;
	int			plugins;

	struct config		config;
	struct config_info	info;
} CONFIGBUTTON;


CONFIGBUTTON *cb;

#include		"load_plugins.h"


#endif
