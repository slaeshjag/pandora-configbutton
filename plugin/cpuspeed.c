#include "../include/configbutton.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

typedef struct {
	int		setspeed;
} INTERNAL;

static void read_values();

static int		values[5];
static char		confbuff[1024];

const char plugin_name[] = "CPU speed";
const char plugin_desc[] = "Adds a menu of CPU speed presets";

static GtkWidget *presets[5];


static void saveSpeeds(GtkWidget *w, gpointer null) {
	fprintf(stderr, "TODO: implement\n");
}


void configure() {
	GtkWidget *win, *vbox, *hbox, *b;
	int i;

	read_values();
	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(win), "CPU speed settings");

	vbox = gtk_vbox_new(FALSE, 0);

	for (i = 0; i < 5; i++) {
		hbox = gtk_hbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
		presets[i] = gtk_spin_button_new_with_range(0., 5000., 1.);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(presets[i]), values[i]);
		gtk_box_pack_start(GTK_BOX(hbox), presets[i], TRUE, TRUE, 5);
		b = gtk_label_new("MHz");
		gtk_box_pack_start(GTK_BOX(hbox), b, FALSE, FALSE, 5);
	}
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
	b = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox), b, TRUE, TRUE, 5);
	b = gtk_button_new_from_stock(GTK_STOCK_REVERT_TO_SAVED);
	gtk_box_pack_start(GTK_BOX(hbox), b, FALSE, FALSE, 5);
	b = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_box_pack_start(GTK_BOX(hbox), b, FALSE, FALSE, 5);
	b = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_box_pack_start(GTK_BOX(hbox), b, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(saveSpeeds), NULL);

	

	gtk_container_add(GTK_CONTAINER(win), vbox);

	gtk_widget_show_all(win);


	#if 0
	d = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "CPU speed config dialog not yet implemented");
	gtk_dialog_run(GTK_DIALOG(d));
	gtk_widget_destroy(d);
	#endif

	return;
}


int activate(void *internal) {
	char cpucmd[256];
	INTERNAL *ip = internal;

	if (ip->setspeed == 0)
		sprintf(cpucmd, "sudo -n /usr/pandora/scripts/op_cpuspeed.sh");
	else
		sprintf(cpucmd, "sudo -n /usr/pandora/scripts/op_cpuspeed.sh %i", ip->setspeed);

	system(cpucmd);
	return 0;
}


int getMinCPU() {
	int number;
	char *mhz;

	if ((mhz = strstr(confbuff, "min:")) == NULL) {
		fprintf(stderr, "Error: Unable to find MIN CPU-speed setting\n");
		return 600;
	}

	mhz += 4;
	sscanf(mhz, "%i", &number);

	return (number < 600) ? 600 : number;
}


int getMaxCPU() {
	FILE *fp;
	int number;
	char *mhz;
	
	*confbuff = 0;
	if ((fp = fopen("/etc/pandora/conf/cpu.conf", "r")) == NULL) {
		fprintf(stderr, "Error: Unable to load /etc/pandora/conf/cpu.conf! Assuming 800 MHz...");
		return 800;
	}

	confbuff[fread(confbuff, 1, 1024, fp)] = 0;
	fclose(fp);

	if ((mhz = strstr(confbuff, "max:")) == NULL) {
		fprintf(stderr, "Unable to find MAX CPU-speed setting\n");
		return 800;
	}
	mhz += 4;
	sscanf(mhz, "%i", &number);

	return number;
}


int getStep(int mhz, int min) {
	int i, j;

	for (i = 1, j = mhz; j > 5; i++) 
		j = (mhz - min + i * 25) / (i*25);
	return (i-1) * 25;
}


static void read_values() {
	int min, max, loops, speed, step, i;
	char path[520];
	FILE *fp;
	
	sprintf(path, "%s/.config-button/cpu-speed.conf", getenv("HOME"));
	fp = fopen(path, "r");
	
	max = getMaxCPU();
	min = getMinCPU();
	step = getStep(max, min);
	loops = (max - min) / step;
	
	for (i = loops; i >= 0; i--) {
		if (fp) {
			speed = 0;
			fscanf(fp, "%i\n", &speed);
		} else
			speed = min + i * step;
		values[i] = speed;
	}

	if (fp)
		fclose(fp);
	return;
}


int getinfo(PLUGIN_INFO *info) {
	int i;
	struct PLUGIN_SUBMENU *sub;
	char cpuspeed[10];
	FILE *fp;
	char *nnn;
	INTERNAL *internal;

	if (info->submenu == NULL) {
		read_values();
		internal = malloc(sizeof(INTERNAL));
		info->label = malloc(128);
		
		for (i = 4; i >= 0; i--) {
			if (!values[i])
				continue;
			sub = malloc(sizeof(struct PLUGIN_SUBMENU));
			sub->next = info->submenu;
			nnn = malloc(32);
			
			sprintf(nnn, "%i MHz", values[i]);
			sub->label = nnn;
			sub->icon_path = "/usr/share/icons/pandora/cpu.png";
			sub->visible = 1;
			internal = malloc(sizeof(INTERNAL));
			internal->setspeed = values[i];
			sub->internal = internal;
			info->submenu = sub;
		}
		
		sub = malloc(sizeof(struct PLUGIN_SUBMENU));
		internal = malloc(sizeof(INTERNAL));
		sub->next = info->submenu;
		sub->label = "Custom";
		sub->icon_path = "/usr/share/icons/pandora/cpu.png";
		sub->visible = 1;
		internal->setspeed = 0;
		sub->internal = internal;
		info->submenu = sub;
	}

	fp = fopen("/proc/pandora/cpu_mhz_max", "r");
	if (fp != NULL) {
		fscanf(fp, "%s", cpuspeed);
		fclose(fp);
	} else
		sprintf(cpuspeed, "N/A");
	sprintf((char *) info->label, "CPU Speed (Cur: %s MHz)", cpuspeed);
	info->icon_path = "/usr/share/icons/pandora/cpu.png";
	
	info->sort_hint = 0;

	return 0;
}
