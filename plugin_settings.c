#include "configbutton.h"



void settingsWindowNew() {
	GtkWidget *win, *vbox, *hbox, *label;

	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(win), 10);
	gtk_widget_set_size_request(win, 600, 250);
	gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(win), "Plugin settings");

	hbox = gtk_hbox_new(FALSE, 0);

	/* Disabled plugins */
	vbox = gtk_vbox_new(FALSE, 0);
	label = gtk_label_new("Disabled plugins");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);

	gtk_container_add(GTK_CONTAINER(hbox), vbox);

	/* TODO: Add column for enabled plugins */
	vbox = gtk_vbox_new(FALSE, 0);
	label = gtk_label_new("Enabled plugins");
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
	gtk_container_add(GTK_CONTAINER(hbox), vbox);


	
	gtk_container_add(GTK_CONTAINER(win), hbox);
	gtk_widget_show_all(win);

	return;
}
