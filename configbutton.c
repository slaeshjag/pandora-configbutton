#include "configbutton.h"


void configbutton_MakeUsAlone() {
	FILE *pidfile, *exec;
	int pid_kill;
	pid_t pid;
	char path[128];

	pid_kill = 0;

	if ((pidfile = fopen("/tmp/configbutton.pid", "r")) != NULL) {
		fscanf(pidfile, "%i", &pid_kill);
		if (pid_kill < 1);
		else {
			sprintf(path, "/proc/%i/cmdline", pid_kill);
			fprintf(stderr, "Opening %s...\n", path);
			if ((exec = fopen(path, "r")) != NULL) {
				fgets(path, 128, exec);
				if (strstr(path, "configbutton") != NULL)
					kill(pid_kill, 15);
				else fprintf(stderr, "PID %i doesn't seem to be a configbutton: %s\n", pid_kill, path);
				fclose(exec);
			}
		}
		fclose(pidfile);
	}

	pid = getpid();

	if ((pidfile = fopen("/tmp/configbutton.pid", "w")) == NULL) {
		fprintf(stderr, "ERROR: Unable to create /tmp/configbutton.pid\n");
		exit(-1);
	}

	fprintf(pidfile, "%i", pid);
	fclose(pidfile);

	return;
}


void configbuttonActivate(GtkWidget *menu_item, gpointer data) {
	PLUGIN_STRUCT *plugin = data;

	(plugin->activate)(plugin->info->internal);

	return;
}


void configbuttonActivateSubmenu(GtkWidget *menu_item, gpointer data) {
	struct PLUGIN_SUB_ENTRY *plugin = data;

	(plugin->activate)(plugin->internal);

	return;
}


void configbuttonCreateMenu(CONFIGBUTTON *c) {
	struct PLUGIN_SUB_ENTRY *plugin_sub;
	int i;
	GtkWidget *image;
	GtkWidget *submenu;
	
	c->menu = gtk_menu_new();
	updatePlugins(c);
	gtk_status_icon_set_from_stock(c->icon, GTK_STOCK_PREFERENCES);

	for (i = 0; i < c->plugins; i++) {
		c->plugin[i].item = gtk_image_menu_item_new_with_label(c->plugin[i].info->label);
		if (c->plugin[i].info->icon_path != NULL) {
			image = gtk_image_new_from_file(c->plugin[i].info->icon_path);
			gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(c->plugin[i].item), image);
		}
		
		if (c->plugin[i].info->submenu == NULL)
			g_signal_connect(G_OBJECT(c->plugin[i].item), "activate", G_CALLBACK(configbuttonActivate), &c->plugin[i]);
		else {
			plugin_sub = c->plugin[i].info->submenu;
			submenu = gtk_menu_new();
			while (plugin_sub != NULL) {
				if (plugin_sub->visible == 0) {
					plugin_sub = plugin_sub->next;
					continue;
				}
				plugin_sub->item = gtk_image_menu_item_new_with_label(plugin_sub->label);
				if (plugin_sub->icon_path != NULL) {
					image = gtk_image_new_from_file(plugin_sub->icon_path);
					gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(plugin_sub->item), image);
				}
				g_signal_connect(G_OBJECT(plugin_sub->item), "activate", G_CALLBACK(configbuttonActivateSubmenu), plugin_sub);
				gtk_menu_shell_append(GTK_MENU_SHELL(submenu), plugin_sub->item);
				plugin_sub->activate = c->plugin[i].activate;
				plugin_sub = plugin_sub->next;
			}
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(c->plugin[i].item), submenu);
		}
		gtk_menu_shell_append(GTK_MENU_SHELL(c->menu), c->plugin[i].item);
	}
			

	gtk_widget_show_all(c->menu);
	
	return;
}


void configbuttonDestroyMenu(CONFIGBUTTON *c) {
	gtk_widget_destroy(c->menu);

	return;
}


void configbuttonSpawnMenu(GtkWidget *icon, gpointer data) {
	CONFIGBUTTON *c = data;


	if (c->menu != NULL)
		configbuttonDestroyMenu(c);

	configbuttonCreateMenu(c);
	gtk_menu_popup(GTK_MENU(c->menu), NULL, NULL, gtk_status_icon_position_menu, c->icon, 0, gtk_get_current_event_time());

	return;
}


int configbuttonInit(CONFIGBUTTON *c) {
	c->icon = gtk_status_icon_new_from_stock(GTK_STOCK_PREFERENCES);

	g_signal_connect(G_OBJECT(c->icon), "activate", G_CALLBACK(configbuttonSpawnMenu), c);
	c->menu = NULL;
	initPlugins(c);

	return 0;
}


int main(int argc, char **argv) {
	CONFIGBUTTON *c;

	if ((c = malloc(sizeof(CONFIGBUTTON))) == NULL) {
		fprintf(stderr, "Unable to malloc(%i), probably out of RAM\n", (int) sizeof(CONFIGBUTTON));
		return -1;
	}

	gtk_init(&argc, &argv);
	if (configbuttonInit(c) < 0)
		return -1;

	configbutton_MakeUsAlone();
	gtk_main();

	return 0;
}
