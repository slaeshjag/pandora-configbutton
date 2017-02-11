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
const char plugin_desc[] = "Adds a menu for connecting to bluetooth headsets";
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
	char mac[17];
	pDevice next;
} tDevice;

static pDevice first_device = NULL;

static void cleanDevices(pDevice *first)
{
	while (*first)
	{
		pDevice next = (*first)->next;
		free(*first);
		*first = next;
	}
}

#define CHECK_AND_ADD_MACRO(s) \
	if (strstr(buffer,s) == NULL) \
	{ \
		sprintf(&(enable[l]),s","); \
		l = strlen(enable); \
	}

#define SUDO_MESSAGE "The sound output plugin needs super user rights to setup the audio environment."

static void checkGlobalSettings()
{
	char buffer[1024];
	char enable[1024] = "";
	FILE* p;
	int l = 0;
	sprintf(buffer," cat /etc/bluetooth/audio.conf | grep \"Enable=\"");
	p = popen(buffer, "r");
	buffer[fread(buffer, 1, 1024, p)] = 0;
	pclose(p);
	CHECK_AND_ADD_MACRO("Source")
	CHECK_AND_ADD_MACRO("Sink")
	CHECK_AND_ADD_MACRO("Headset")
	CHECK_AND_ADD_MACRO("Socket")
	CHECK_AND_ADD_MACRO("Control")
	if (enable[0])
	{
		sprintf(buffer,"gksudo --message \""SUDO_MESSAGE"\" \"sed -i.soundoutput.backup 's/Enable=/Enable=%s/' /etc/bluetooth/audio.conf\"",enable);
		printf("%s\n",buffer);
		system(buffer);
	}
	sprintf(buffer,"cat /usr/share/alsa/alsa.conf | grep -q \"~/.asoundrc\"");
	if (system(buffer) == 0)
	{
		sprintf(buffer,"gksudo --message \""SUDO_MESSAGE"\" \"sed -i.soundoutput.backup 's,~/.asoundrc,/home/\\${USER}/.asoundrc,' /usr/share/alsa/alsa.conf\"");
		printf("%s\n",buffer);
		system(buffer);
	}
	p = fopen("/etc/asound.conf", "r");
	if (p)
	{
		fclose(p);
		sprintf(buffer,"gksudo --message \""SUDO_MESSAGE"\" \"mv /etc/asound.conf /etc/asound.conf.soundoutput.backup\"");
		printf("%s\n",buffer);
		system(buffer);
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
	cleanDevices(&first_device);
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

pDevice getBluetoothDevices()
{
	pDevice first = NULL;
	char filename[1024];
	DIR* d = opendir("/var/lib/bluetooth");
	if (d)
	{
		struct dirent *dir;
		while ((dir = readdir(d)) != NULL)
		{
			if (strcmp(dir->d_name,".") == 0 || strcmp(dir->d_name,"..") == 0)
				continue;
			sprintf(filename,"/var/lib/bluetooth/%s/names",dir->d_name);
			FILE* fp = fopen(filename, "r");
			if (fp)
			{
				pDevice dev = (pDevice)malloc(sizeof(tDevice));
				while (fscanf(fp, "%s %[^\n]", dev->mac, dev->name) > 1)
				{
					dev->next = first;
					first = dev;
					dev = (pDevice)malloc(sizeof(tDevice));
				}
				free(dev);
				fclose(fp);
			}
		}
		closedir(d);
	}
	return first;
}

GtkWidget *win;
GtkListStore *model;
GtkWidget *lbox;
GtkWidget *add_name, *add_mac;
GtkWidget *win_edit;
GtkTextBuffer *tb;

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
	sprintf(buffer,"sed -i 's,PAS_PATH=/home/${USER}/pas,PAS_PATH=%s,' %s/pas.sh",path,path);
	system(buffer);
	
	updateDevices(0);
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
	gtk_widget_destroy(win);
}

static pDevice getDeviceAndNr(GtkTreeView* l, int* id,pDevice* prev)
{
	GList* list;
	GtkTreeSelection *s = gtk_tree_view_get_selection( GTK_TREE_VIEW( l ) );
	if (gtk_tree_selection_count_selected_rows( s ) <= 0 )
		return NULL;	
	list = gtk_tree_selection_get_selected_rows( s, NULL );
	GtkTreePath* path = g_list_first(list)->data;
	*id = gtk_tree_path_get_indices( path )[0];
	pDevice dev = first_device;
	if (prev)
		*prev = NULL;
	int i = 0;
	while (dev)
	{
		if (i == *id)
			break;
		i++;
		if (prev)
			*prev = dev;
		dev = dev->next;
	}
	g_list_foreach( list, (GFunc) gtk_tree_path_free, NULL );
	g_list_free( list );
	return dev;
}

static void removeDevice(GtkWidget *b, gpointer *l)
{
	GtkWidget *d;
	GtkTreeIter iter;
	char filename[1024];
	int id;
	pDevice prev, dev = getDeviceAndNr((GtkTreeView*)l, &id, &prev);
	if (dev)
	{
		char buffer[256];
		sprintf(buffer,"Do you really want to remove \"%s\"? This cannot be undone!",dev->name);
		d = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, buffer);
		if (gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_YES)
		{
			if (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(model),&iter,NULL,id))
				gtk_list_store_remove(model,&iter);
			if (prev)
				prev->next = dev->next;
			else
				first_device = dev->next;
			sprintf(filename,"%s/"PREFIX"%s",pas_path,dev->name);
			remove(filename);
			free(dev);
		}
		gtk_widget_destroy(d);
	}
}

pDevice findDevice(pDevice first,const char* name)
{
	pDevice dev = first;
	while (dev)
	{
		if (strcmp(name,dev->name) == 0)
			break;
		dev = dev->next;
	}
	return dev;
}


static void addDevice(GtkWidget *b, gpointer *__window)
{
	GtkWidget *d;
	GtkWidget *window = GTK_WIDGET(__window);
	const gchar* name = gtk_entry_get_text(GTK_ENTRY(add_name));
	char buffer[256] = "";
	if (findDevice(first_device,name))
		sprintf(buffer,"Device name \"%s\" already in use!",name);
	if (strcmp(name,"bt") == 0)
		sprintf(buffer,"Device name \"%s\" not alowed (would overwrite template file)!",name);
	if (strchr(name,' '))
		sprintf(buffer,"Device name \"%s\" not alowed because of spaces!",name);
	if (strchr(name,'\"'))
		sprintf(buffer,"Device name \"%s\" not alowed because of quotes!",name);
	if (strchr(name,'(') || strchr(name,')'))
		sprintf(buffer,"Device name \"%s\" not alowed because of parantheses!",name);
	if (strchr(name,'*'))
		sprintf(buffer,"Device name \"%s\" not alowed because of asterisks!",name);
	if (buffer[0])
	{
		d = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, buffer);
		gtk_dialog_run(GTK_DIALOG(d));
		gtk_widget_destroy(d);
		return;
	}
	const gchar* mac = gtk_entry_get_text(GTK_ENTRY(add_mac));
	char cmd[1024];
	sprintf(cmd, "sed 's/device \"XX:XX:XX:XX:XX:XX\"/device \"%s\"/'  %s/.asoundrc_bt > %s/.asoundrc_%s", mac, pas_path, pas_path, name );
	system(cmd);
	updateDevices(1);
	gtk_widget_destroy(GTK_WIDGET(window));
}

static void saveText(GtkWidget *b, gpointer *name)
{
	GtkTextIter start, end;
	char buffer[1024];
	gtk_text_buffer_get_start_iter( tb, &start );
	gtk_text_buffer_get_end_iter( tb , &end );
	gchar* file_buffer = gtk_text_buffer_get_text( tb , &start, &end, FALSE );
	sprintf( buffer, "%s/.asoundrc_%s",pas_path,(char*)name);
	g_file_set_contents (buffer,file_buffer,-1, NULL);
	g_free(file_buffer);
	gtk_widget_destroy(GTK_WIDGET(win_edit));
}

static void closeWindow(GtkWidget *w, gpointer window)
{
	gtk_widget_destroy(GTK_WIDGET(window));
}

gboolean view_selection_func(
	GtkTreeSelection *selection,
	GtkTreeModel     *model,
	GtkTreePath      *path,
	gboolean          path_currently_selected,
	gpointer          null)
{
	GtkTreeIter iter;
	if (!path_currently_selected && gtk_tree_model_get_iter(model, &iter, path))
	{
		gchar *mac;
		gchar *name;
		gtk_tree_model_get(model, &iter, 0, &mac, -1);
		gtk_tree_model_get(model, &iter, 1, &name, -1);
		int i;
		for (i=0;name[i];++i)
			if (name[i] == ' ' || name[i] == '\"' ||
				name[i] == '(' || name[i] == ')'  ||
				name[i] == '*')
				name[i] = '_';
		gtk_entry_set_text(GTK_ENTRY(add_mac), mac );
		gtk_entry_set_text(GTK_ENTRY(add_name), name );
		g_free(mac);
		g_free(name);
	}
	return TRUE;
}

static void addDeviceDialog(GtkWidget *w, gpointer null)
{
	checkGlobalSettings();
	GtkWidget *win_add, *vbox, *hbox, *b, *sbox;
	GtkCellRenderer *renderer;
	GtkTreeIter iter;
	pDevice first_bt = getBluetoothDevices();
	win_add = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(win_add), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(win_add), "Add new (bluetooth) device");
	gtk_window_set_modal(GTK_WINDOW(win_add),TRUE);
	gtk_window_set_default_size(GTK_WINDOW(win_add),-1,300);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(win_add), vbox);
	b = gtk_label_new("Enter the MAC adress of the bluetooth device and a name either by hand or from the list below. You can later further edit the .asoundrc_ file in the sound output plugin settings if needed.");
	gtk_label_set_line_wrap(GTK_LABEL(b), TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), b, FALSE, FALSE, 5);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
	add_mac = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), add_mac, FALSE, FALSE, 5);
	gtk_entry_set_text(GTK_ENTRY(add_mac), "XX:XX:XX:XX:XX:XX" );
	gtk_entry_set_max_length(GTK_ENTRY(add_mac), 17 );
	add_name = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), add_name, FALSE, FALSE, 5);
	gtk_entry_set_text(GTK_ENTRY(add_name), "name" );
	gtk_entry_set_max_length(GTK_ENTRY(add_mac), 64 );
	b = gtk_button_new_from_stock(GTK_STOCK_ADD);
	g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(addDevice), win_add);
	gtk_box_pack_start(GTK_BOX(hbox), b, FALSE, FALSE, 5);
	b = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	gtk_box_pack_start(GTK_BOX(hbox), b, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(closeWindow), win_add);

	sbox = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), sbox, TRUE, TRUE, 5);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sbox), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );

	hbox = gtk_tree_view_new();
	gtk_container_add(GTK_CONTAINER(sbox), hbox);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(
		GTK_TREE_VIEW(hbox),
		-1,
		"MAC",
		renderer,
		"text",
		0,
		NULL);
	gtk_tree_view_insert_column_with_attributes(
		GTK_TREE_VIEW(hbox),
		-1,
		"Name",
		renderer,
		"text",
		1,
		NULL);
	model = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_STRING );
	pDevice bt_dev = first_bt;
	while (bt_dev)
	{
		gtk_list_store_append( model, &iter );
		gtk_list_store_set( model, &iter, 0, bt_dev->mac, 1, bt_dev->name, -1 );
		bt_dev = bt_dev->next;
	}
	cleanDevices(&first_bt);
	gtk_tree_view_set_model( GTK_TREE_VIEW( hbox ), GTK_TREE_MODEL( model ) );
	gtk_tree_selection_set_select_function(gtk_tree_view_get_selection(GTK_TREE_VIEW(hbox)), view_selection_func, NULL, NULL);	
	gtk_widget_show_all(win_add);
}

static void editDeviceDialog(GtkWidget *w, gpointer l)
{
	GtkWidget *vbox, *hbox, *b, *sbox;
	char buffer[1024];
	int id;
	gchar *file_buffer;
	pDevice prev, dev = getDeviceAndNr((GtkTreeView*)l, &id, &prev);
	if (!dev)
		return;
	win_edit = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(win_edit), GTK_WIN_POS_CENTER);
	sprintf(buffer, "Edit audio device \"%s\"",dev->name);
	gtk_window_set_title(GTK_WINDOW(win_edit), buffer);
	gtk_window_set_modal(GTK_WINDOW(win_edit),TRUE);
	gtk_window_set_default_size(GTK_WINDOW(win_edit),600,420);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(win_edit), vbox);
	b = gtk_label_new("Edit the asoundrc_ file for your needs. Keep in mind, there is now correctness check, so you may make the config file invalid.");
	gtk_label_set_line_wrap(GTK_LABEL(b), TRUE);
	gtk_widget_set_size_request( b, 600, -1);
	gtk_box_pack_start(GTK_BOX(vbox), b, FALSE, FALSE, 5);

	sbox = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), sbox, TRUE, TRUE, 5);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sbox), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	b = gtk_text_view_new();
	gtk_container_add(GTK_CONTAINER(sbox), b);
	gtk_widget_modify_font( b, pango_font_description_from_string("MONOSPACE"));
	tb = gtk_text_view_get_buffer( GTK_TEXT_VIEW(b));
	sprintf( buffer, "%s/.asoundrc_%s",pas_path,dev->name);
	if (g_file_get_contents (buffer,&file_buffer,NULL, NULL))
	{
		gtk_text_buffer_set_text( tb, file_buffer, -1 );
		g_free(file_buffer);
	}
	else
		gtk_text_buffer_set_text( tb, "ERROR: Could not load config!", -1 );

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
	b = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox), b, TRUE, TRUE, 5);
	b = gtk_button_new_with_mnemonic("_Discard");
	gtk_button_set_image(GTK_BUTTON(b), gtk_image_new_from_stock( GTK_STOCK_CANCEL, GTK_ICON_SIZE_BUTTON ) );
	g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(closeWindow), win_edit);
	gtk_box_pack_start(GTK_BOX(hbox), b, FALSE, FALSE, 5);
	b = gtk_button_new_from_stock(GTK_STOCK_SAVE);
	gtk_box_pack_start(GTK_BOX(hbox), b, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(saveText), dev->name);

	gtk_widget_show_all(win_edit);
}

void set_pas_path()
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
}

void configure()
{
	checkGlobalSettings();
	set_pas_path();
	GtkWidget *vbox, *hbox, *b, *e, *subvbox, *sbox;
	GtkCellRenderer *renderer;
	GtkTreeIter iter;

	updateDevices(1);
	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(win), "Sound output settings");
	gtk_window_set_modal(GTK_WINDOW(win),TRUE);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(win), vbox);
	b = gtk_label_new("Path to \"pas.sh\". If the folder does not contain \"pas.sh\", version 0.4 will be installed in it.");
	gtk_label_set_line_wrap(GTK_LABEL(b), TRUE);
	gtk_widget_set_size_request( b, 400, -1);
	gtk_box_pack_start(GTK_BOX(vbox), b, FALSE, FALSE, 5);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
	e = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), e, FALSE, FALSE, 5);
	gtk_entry_set_text(GTK_ENTRY(e), pas_path );
	gtk_entry_set_max_length(GTK_ENTRY(e), 520 );
	gtk_entry_set_width_chars(GTK_ENTRY(e), 35 );
	b = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	gtk_box_pack_start(GTK_BOX(hbox), b, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(applyPath), e);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
	sbox = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(hbox), sbox, TRUE, TRUE, 5);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sbox), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	lbox = gtk_tree_view_new();
	gtk_container_add(GTK_CONTAINER(sbox), lbox);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(
		GTK_TREE_VIEW(lbox),
		-1,
		"Sound output device name",
		renderer,
		"text",
		0,
		NULL);
	model = gtk_list_store_new( 1, G_TYPE_STRING );
	pDevice dev = first_device;
	while (dev)
	{
		gtk_list_store_append( model, &iter );
		gtk_list_store_set( model, &iter, 0, dev->name, -1 );
		dev = dev->next;
	}
	gtk_tree_view_set_model( GTK_TREE_VIEW( lbox ), GTK_TREE_MODEL( model ) );
	//buttons on the right	
	subvbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), subvbox, FALSE, FALSE, 5);
	b = gtk_button_new_with_mnemonic("_Edit directly");
	gtk_button_set_image(GTK_BUTTON(b), gtk_image_new_from_stock( GTK_STOCK_EDIT, GTK_ICON_SIZE_BUTTON ) );
	gtk_box_pack_start(GTK_BOX(subvbox), b, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(editDeviceDialog), lbox );
	b = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
	g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(removeDevice), lbox );
	gtk_box_pack_start(GTK_BOX(subvbox), b, FALSE, FALSE, 5);
	b = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	gtk_box_pack_start(GTK_BOX(subvbox), b, FALSE, FALSE, 5);
	g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(closeWindow), win);
	g_object_unref( model );
	
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
		addDeviceDialog(NULL,NULL);
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
	set_pas_path();
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
