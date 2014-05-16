#include "../include/configbutton.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


const char plugin_name[] = "Execute command";
const char plugin_desc[] = "Adds a menu of user-defined shell commands";

#define	PRIMARY_ICON	"/usr/share/icons/pandora/exec_command.png"
static GtkWidget *win, *list, *cmd, *icon, *name;

struct entry {
	char		*command;
	char		*icon;
	char		*name;
};


struct internal {
	struct entry	*entry;
	int		entries;
};


int activate(void *internal) {
	system(internal);

	return 0;
}


static void init_list(GtkWidget *list) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkListStore *store;

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Commands", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

	store = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));

	g_object_unref(store);

	return;
}


void list_add(GtkWidget *list, const gchar *str) {
	GtkListStore *store;
	GtkTreeIter iter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, str, -1);
}


void list_updated(GtkWidget *widget, gpointer null) {
	return;
}


static void new_list(GtkWidget *vbox, GtkWidget **list) {
	GtkWidget *scrollwin;

	scrollwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	*list = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(*list), FALSE);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollwin), *list);
	gtk_box_pack_start(GTK_BOX(vbox), scrollwin, TRUE, TRUE, 5);

	g_signal_connect(gtk_tree_view_get_selection(GTK_TREE_VIEW((*list))), "changed", G_CALLBACK(list_updated), NULL);

	return;
}


void configure() {
	GtkWidget *whbox, *table;
	/* TODO: load in commands */
	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(win), "Execute command settings - Config tray");
	
	whbox = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(win), whbox);
	
	new_list(whbox, &list);
	init_list(list);
	
	table = gtk_table_new(2, 4, TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 2);
	gtk_table_set_col_spacings(GTK_TABLE(table), 2);
	gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new("Name"), 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new("Icon"), 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new("Command"), 0, 1, 2, 3);
	gtk_box_pack_start(GTK_BOX(whbox), table, FALSE, FALSE, 5);


	

	gtk_widget_show_all(win);
}


int getinfo(PLUGIN_INFO *info) {
	struct internal *in;
	int i;
	char path[PATH_MAX], buff_name[64], buff_icon[PATH_MAX], buff_command[2048];
	FILE *fp;
	struct PLUGIN_SUBMENU *sub;

	in = info->internal;
	if (!info->internal) {
		sprintf(path, "%s/.config-button/commands.presets", getenv("HOME"));
		if (!(fp = fopen(path, "r"))) {
			fprintf(stderr, "Unable to load commands\n");
			return -1;
		}
		

		in = malloc(sizeof(*in));
		in->entry = NULL;
		in->entries = 0;

		for (i = 0; !feof(fp); i++) {
			*buff_name = *buff_icon = *buff_command = 0;
			fscanf(fp, "%[^\t\n] %[^\t\n] %[^\t\n]\n", buff_name, buff_icon, buff_command);
			if (!(*buff_name))
				break;
			in->entry = realloc(in->entry, (i + 1) * sizeof(*in->entry));
			in->entries = i + 1;
			in->entry[i].command = strdup(buff_command);
			if (*buff_icon)
				in->entry[i].icon = strdup(buff_icon);
			else
				in->entry[i].icon = strdup(PRIMARY_ICON);
			in->entry[i].name = strdup(buff_name);
		}

		info->internal = in;

		info->label = "Execute command";
		info->icon_path = PRIMARY_ICON;
		info->sort_hint = 5;

		for (i = in->entries - 1; i >= 0; i--) {
			sub = info->submenu;
			info->submenu = malloc(sizeof(*info->submenu));
			info->submenu->next = sub;
			info->submenu->label = in->entry[i].name;
			info->submenu->internal = in->entry[i].command;
			info->submenu->icon_path = in->entry[i].icon;
			info->submenu->visible = 1;
			info->submenu->activate = NULL;
		}
	}

	return 0;
}
