#include "../include/configbutton.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "pas.h" //VERSION 0.4 as base64

const char plugin_name[] = "Sound Output";
const char plugin_desc[] = "Adds a menu for connecting to bluetooth head sets";
static char pas_path[520];
static char recent_device[64] = "unknown";

typedef enum
{
	ADD,
	CARD
} INTERNAL_type;

typedef struct
{
	INTERNAL_type type;
	char name[64];
} INTERNAL;

typedef struct sDevice *pDevice;
typedef struct sDevice
{
	char name[64];
	pDevice next;
} tDevice;

static pDevice first_device = NULL;

static void cleanDevices()
{
	while (first_device)
	{
		pDevice next = first_device->next;
		free(first_device);
		first_device = next;
	}
}

int prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

#define PREFIX ".asoundrc_"
static void updateDevices(int recent_found)
{
	char cmd[1024];
	cleanDevices();
	DIR* d = opendir(pas_path);
	if (d)
	{
		struct dirent *dir;
		while ((dir = readdir(d)) != NULL)
		{
			if (strcmp(dir->d_name,".") == 0 || strcmp(dir->d_name,"..") == 0)
				continue;
			if (strcmp(dir->d_name,PREFIX"bt") == 0)
				continue;
			if (prefix(PREFIX,dir->d_name))
			{
				pDevice dev = (pDevice)malloc(sizeof(tDevice));
				if (snprintf(dev->name,64,"%s",&(dir->d_name[strlen(PREFIX)])) > 63)
					dev->name[63] = 0;
				if (!recent_found)
				{
					sprintf(cmd, "diff /home/${USER}/.asoundrc %s/%s", pas_path, dir->d_name);
					if (system(cmd) == 0)
					{
						sprintf( recent_device, "%s", dev->name );
						recent_found = 1;
					}
				}
				dev->next = first_device;
				first_device = dev;
			}
		}
		closedir(d);
	}
	if (!recent_found)
		sprintf(recent_device,"unknown");
}

GtkWidget *win;

static void extract_pas(const char* path)
{
	char buffer[2048];
	FILE* p;
	const int l = strlen(PAS_ZIP);

	sprintf(buffer,"mkdir -p %s",path);
	system(buffer);
	sprintf(buffer,"base64 -d > %s/pas_temp.zip",path);
	p = popen(buffer, "w");
	fwrite(PAS_ZIP, 1, l, p);
	pclose(p);
	//sprintf(buffer,"wget --no-check-certificate -O %s/pas.zip https://pyra-handheld.com/boards/attachments/pas_0-4-zip.28143/",pas_path);
	//system(buffer);
	sprintf(buffer,"unzip %s/pas_temp.zip -d %s",path,path);
	system(buffer);
	sprintf(buffer,"rm -f %s/pas_temp.zip",path);
	system(buffer);
	//replace PAS_PATH=/home/${USER}/pas/ with choosen path
	sprintf(buffer,"sed -ie 's,PAS_PATH=/home/${USER}/pas,PAS_PATH=%s,' %s/pas.sh",path,path);
	system(buffer);
	
	updateDevices(1);
}

static void messageBox( char* message )
{
	GtkWidget *d;
	d = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, message);
	gtk_dialog_run(GTK_DIALOG(d));
	gtk_widget_destroy(d);
}

static void applyPath(GtkWidget *b, gpointer *e)
{
	char path[1024];
	FILE *fp;
	sprintf(pas_path,"%s", gtk_entry_get_text(GTK_ENTRY(e)));
	sprintf(path, "%s/.config-button/sound-output.conf", getenv("HOME"));
	fp = fopen(path, "w");
	if (fp)
	{
		fprintf(fp, "%s\n", pas_path);
		fclose(fp);
	}
	//Create pas.sh script if needed
	sprintf(path,"%s/pas.sh",pas_path);
	fp = fopen(path, "r");
	if (fp)
		fclose(fp);
	else
	{
		extract_pas(pas_path);
		messageBox( "Created \"pas.sh\" at location" );
	}

}

static void closeWindow(GtkWidget *w, gpointer null)
{
	gtk_widget_destroy(win);
}

void configure()
{
	GtkWidget *vbox, *hbox, *b, *e;
	updateDevices(1);
	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(win), "Sound output settings");
	gtk_window_set_modal(GTK_WINDOW(win),TRUE);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(win), vbox);
	b = gtk_label_new("Path to \"pas.sh\". If it is not found, version 0.4 will installed automaticly:");
	gtk_box_pack_start(GTK_BOX(vbox), b, FALSE, FALSE, 5);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
	e = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), e, FALSE, FALSE, 5);
	gtk_entry_set_text(GTK_ENTRY(e), pas_path );
	gtk_entry_set_max_length(GTK_ENTRY(e), 520 );
	b = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	gtk_box_pack_start(GTK_BOX(hbox), b, FALSE, FALSE, 5);
	gtk_entry_set_width_chars(GTK_ENTRY(e), 44 );
	g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(applyPath), e);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
	b = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	gtk_box_pack_start(GTK_BOX(hbox), b, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(closeWindow), NULL);

	gtk_widget_show_all(win);
	return;
}

int activate(void *internal)
{
	INTERNAL *ip = internal;
	char buffer[1024];
	FILE *fp;

	if (ip->type == ADD)
	{
		sprintf(buffer,"%s/pas.sh",pas_path);
		fp = fopen(buffer, "r");
		if (fp)
			fclose(fp);
		else
			extract_pas(pas_path);
		updateDevices(1);
		messageBox( "Not implemented yet" );
	}
	else
	{
		sprintf(buffer,"%s/pas.sh -o %s",pas_path,ip->name);
		system(buffer);
		sprintf(buffer,"notify-send -i /usr/share/icons/elementary/devices/48/audiocard.svg \"Sound Control\" \"%s was enabled\"",ip->name);
		system(buffer);
		sprintf(recent_device,"%s",ip->name);
	}
	return 0;
}

INTERNAL* create_submenu(PLUGIN_INFO * info, char * name,INTERNAL_type type)
{
	INTERNAL *internal;
	struct PLUGIN_SUBMENU *sub;
	char * nnn;

	internal = malloc(sizeof(INTERNAL));	
	sprintf(internal->name,"%s",name);
	internal->type = type;
	sub = malloc(sizeof(struct PLUGIN_SUBMENU));
	sub->next = info->submenu;
	nnn = malloc(64);
	memcpy( nnn, internal->name, 64 );
	sub->label = nnn;
	switch (type)
	{
		case ADD:
			sub->icon_path = "/usr/share/icons/elementary/actions/48/add.svg";
			break;
		case CARD:
			sub->icon_path = "/usr/share/icons/elementary/devices/48/audiocard.svg";
			break;
	}
	sub->visible = 1;
	sub->internal = internal;
	info->submenu = sub;
	return internal;
}

int getinfo(PLUGIN_INFO *info)
{
	char path[1024];
	FILE *fp;

	sprintf(path, "%s/.config-button/sound-output.conf", getenv("HOME"));
	fp = fopen(path, "r");
	if (fp)
	{
		fscanf(fp, "%s\n", pas_path);
		fclose(fp);
	}
	else
		sprintf(pas_path, "%s/pas", getenv("HOME"));
	
	if (info->submenu == NULL) //first run
	{
		info->label = malloc(128);
		updateDevices(0);
	}
	while (info->submenu)
	{
		free(info->submenu->label);
		struct PLUGIN_SUBMENU *next = info->submenu->next;
		free(info->submenu);
		info->submenu = next;
	}
	create_submenu( info, "Add device", ADD );
	pDevice dev = first_device;
	while (dev)
	{
		create_submenu( info, dev->name, CARD );		
		dev = dev->next;
	}
	info->icon_path = "/usr/share/icons/elementary/devices/48/audiocard.svg";
	sprintf((char *) info->label, "Sound Output (%s)", recent_device );
	info->sort_hint = 0;
	return 0;
}
