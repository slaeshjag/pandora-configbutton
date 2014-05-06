#include "configbutton.h"


static GtkWidget *plugin_disabled_list;
static GtkWidget *plugin_enabled_list;
static GtkWidget *plugin_description;
static GtkWidget *plugin_settings;
static GtkWidget *win = NULL;
static GtkWidget *about_window = NULL;
int show_large_icons;


void settingsAboutClose(GtkWidget *btn, gpointer window) {
	GtkWidget *win = window;
	gtk_widget_destroy(win);
	about_window = NULL;

	return;
}


void settingsAboutInit(GtkWidget *widget, gpointer null) {
	GtkWidget *hbox, *vbox, *b;

	if (about_window) {
		settingsAboutClose(NULL, about_window);
		return;
	}

	about_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(about_window), 10);
	gtk_widget_set_size_request(about_window, 400, 200);
	gtk_window_set_position(GTK_WINDOW(about_window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(about_window), "About configbutton");

	hbox = gtk_hbox_new(FALSE, 0);
	vbox = gtk_vbox_new(FALSE, 0);

	b = gtk_label_new("Fill this in with useful information");
	gtk_box_pack_start(GTK_BOX(vbox), b, FALSE, FALSE, 5);

	b = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	gtk_box_pack_start(GTK_BOX(vbox), b, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(settingsAboutClose), about_window);

	gtk_container_add(GTK_CONTAINER(about_window), vbox);

	gtk_widget_show_all(about_window);
}



void settingsListInit(GtkWidget *list) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkListStore *store;

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Plugins", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

	store = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));

	g_object_unref(store);

	return;
}


void settingsListAdd(GtkWidget *list, const gchar *str) {
	GtkListStore *store;
	GtkTreeIter iter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, str, -1);

	return;
}


void settingsSelectionChanged(GtkWidget *widget, gpointer null) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	char *value;
	char desc[4096];

	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(widget), &model, &iter)) {
		gtk_tree_model_get(model, &iter, 0, &value, -1);
		sprintf(desc, "%s: %s", value, configFindFound(value));
		gtk_label_set_text(GTK_LABEL(plugin_description), desc);

		if (configFindConfig(value))
			gtk_widget_set_sensitive(plugin_settings, TRUE);
		else
			gtk_widget_set_sensitive(plugin_settings, FALSE);

		g_free(value);
	}

	return;
}


void settingsRefreshList() {
	GtkListStore *store;
	GtkTreeModel *model;
	int i;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(plugin_enabled_list));
	store = GTK_LIST_STORE(model);
	gtk_list_store_clear(store);
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(plugin_disabled_list));
	store = GTK_LIST_STORE(model);
	gtk_list_store_clear(store);
	
	gtk_widget_set_sensitive(plugin_settings, FALSE);


	for (i = 0; i < cb->info.infos; i++)
		if (!cb->info.info[i].loaded)
			settingsListAdd(plugin_disabled_list, cb->info.info[i].name);
		else
			settingsListAdd(plugin_enabled_list, cb->info.info[i].name);
}


void settingsPluginDisable(GtkWidget *widget, gpointer null) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	char *value;
	
	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(gtk_tree_view_get_selection(GTK_TREE_VIEW(plugin_enabled_list))), &model, &iter)) {
		gtk_tree_model_get(model, &iter, 0, &value, -1);
		configSetFoundLoaded(value, 0);
		g_free(value);
		settingsRefreshList();
	}
}


void settingsPluginEnable(GtkWidget *widget, gpointer null) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	char *value;
	
	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(gtk_tree_view_get_selection(GTK_TREE_VIEW(plugin_disabled_list))), &model, &iter)) {
		gtk_tree_model_get(model, &iter, 0, &value, -1);
		configSetFoundLoaded(value, 1);
		g_free(value);
		settingsRefreshList();
	}
}


void settingsDialogCancel(GtkWidget *widget, gpointer null) {
	gtk_widget_destroy(win);
	win = NULL;

	return;
}


void settingsIconToggle(GtkWidget *widget, gpointer null) {
	show_large_icons = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

	return;
}


void settingsPreferences(GtkWidget *widget, gpointer null) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	char *value;
	void (*cfg)();
	
	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(gtk_tree_view_get_selection(GTK_TREE_VIEW(plugin_enabled_list))), &model, &iter)) {
		gtk_tree_model_get(model, &iter, 0, &value, -1);
		if (!(cfg = configFindConfig(value)))
			gtk_widget_set_sensitive(plugin_settings, FALSE);
		else
			cfg();

		g_free(value);
	}
	return;
}


void settingsDialogOK(GtkWidget *widget, gpointer null) {
	char path[128];
	char path_to_exec[PATH_MAX];
	
	cb->large_icons = show_large_icons;
	configSaveLoaded();
	
	sprintf(path, "/proc/%i/exe", getpid());
	path_to_exec[readlink(path, path_to_exec, PATH_MAX)] = 0;
	chdir("/");
	execl(path_to_exec, path_to_exec, "nokill", NULL);
	gtk_exit(0);
	exit(0);

	return;
}

void settingsListAddToWindow(GtkWidget *vbox, GtkWidget **list) {
	GtkWidget *scrollwin;

	scrollwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	*list = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(*list), FALSE);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollwin), *list);
	gtk_box_pack_start(GTK_BOX(vbox), scrollwin, TRUE, TRUE, 5);
	settingsListInit(*list);

	g_signal_connect(gtk_tree_view_get_selection(GTK_TREE_VIEW((*list))), "changed", G_CALLBACK(settingsSelectionChanged), NULL);

	return;
}


void settingsWindowNew() {
	GtkWidget *vbox, *hbox, *label, *wvbox, *wbutton;
	GtkToolItem *button;
	GtkWidget *toolbar;

	if (win) {
		gtk_widget_destroy(win);
		win = NULL;
		return;
	}

	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(win), 10);
	gtk_widget_set_size_request(win, 600, 350);
	gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(win), "Plugin settings");

	wvbox = gtk_vbox_new(FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 0);

	/* Disabled plugins */
	vbox = gtk_vbox_new(FALSE, 0);
	label = gtk_label_new("Disabled plugins");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
	
	settingsListAddToWindow(vbox, &plugin_disabled_list);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 5);

	/* Add enable/disable buttons */
	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	gtk_container_set_border_width(GTK_CONTAINER(toolbar), 2);
	gtk_toolbar_set_orientation(GTK_TOOLBAR(toolbar), GTK_ORIENTATION_VERTICAL);
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(toolbar), GTK_ICON_SIZE_SMALL_TOOLBAR);

	/* pad start */
	button = gtk_tool_item_new();
	gtk_tool_item_set_expand(button, TRUE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

	button = gtk_tool_button_new_from_stock(GTK_STOCK_ADD);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(settingsPluginEnable), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

	button = gtk_tool_button_new_from_stock(GTK_STOCK_REMOVE);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(settingsPluginDisable), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

	/* Pad end */
	button = gtk_tool_item_new();
	gtk_tool_item_set_expand(button, TRUE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

	gtk_box_pack_start(GTK_BOX(hbox), toolbar, FALSE, FALSE, 5);


	/* TODO: Add column for enabled plugins */
	vbox = gtk_vbox_new(FALSE, 0);
	label = gtk_label_new("Enabled plugins");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 5);
	
	settingsListAddToWindow(vbox, &plugin_enabled_list);

	/* Preferences button */
	plugin_settings = gtk_button_new_from_stock(GTK_STOCK_PREFERENCES);
	gtk_box_pack_start(GTK_BOX(vbox), plugin_settings, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(plugin_settings), "clicked", G_CALLBACK(settingsPreferences), NULL);
	gtk_widget_set_sensitive(plugin_settings, FALSE);

	/* Add description label */
	plugin_description = gtk_label_new("");
	gtk_label_set_line_wrap(GTK_LABEL(plugin_description), TRUE);

	settingsRefreshList();

	gtk_container_add(GTK_CONTAINER(win), wvbox);
	gtk_box_pack_start(GTK_BOX(wvbox), hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(wvbox), plugin_description, FALSE, FALSE, 5);

	/**********************************************/
	/********** Add about/apply/cancel buttons **********/

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(wvbox), hbox, FALSE, FALSE, 5);

	wbutton = gtk_button_new_from_stock(GTK_STOCK_ABOUT);
	g_signal_connect(G_OBJECT(wbutton), "clicked", G_CALLBACK(settingsAboutInit), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), wbutton, FALSE, FALSE, 5);

	show_large_icons = cb->large_icons;
	wbutton = gtk_check_button_new_with_label("Show large icons");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wbutton), show_large_icons ? TRUE : FALSE);
	g_signal_connect(G_OBJECT(wbutton), "clicked", G_CALLBACK(settingsIconToggle), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), wbutton, TRUE, TRUE, 5);

	wbutton = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect(G_OBJECT(wbutton), "clicked", G_CALLBACK(settingsDialogCancel), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), wbutton, FALSE, FALSE, 5);

	wbutton = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect(G_OBJECT(wbutton), "clicked", G_CALLBACK(settingsDialogOK), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), wbutton, FALSE, FALSE, 5);
	


	gtk_widget_show_all(win);

	return;
}
