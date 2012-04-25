#ifndef __CONFIGBUTTON_H__
#define	__CONFIGBUTTON_H__


typedef struct {
	const char			*label;
	const char			*icon_path;
	void				*internal;
	struct PLUGIN_SUBMENU		*submenu;
	int				sort_hint;
} PLUGIN_INFO;


struct PLUGIN_SUBMENU {
	void				*gtkmenuentry; // Don't touch this inside the plugin
	const char			*label;
	void				*internal;
	const char			*icon_path;
	int				visible;
	struct PLUGIN_SUBMENU		*next;
	void				*activate;
};

	
	


int getinfo(PLUGIN_INFO *info);
int activate(void *internal);


#endif
