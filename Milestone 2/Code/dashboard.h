#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <gtk/gtk.h>

// Kick off the GTK‐based dashboard
void create_gui();
GtkWidget* create_process_and_scheduler_tab();

// Declare the create_styled_label function
GtkWidget* create_styled_label(const char* text, const char* css_class);

// Declare the on_add_process_clicked callback
void on_add_process_clicked(GtkWidget *widget, gpointer data);

// Declare the on_scheduler_changed callback
void on_scheduler_changed(GtkComboBoxText *scheduler_dropdown, gpointer quantum_entry);

// Declare the on_initialize_process_clicked callback
void on_initialize_process_clicked(GtkWidget *widget, gpointer user_data);

// Declare the initializeProcess function
void initializeProcess(const char* filename, int priority, int arrival_time);

// Declare the setSchedulerName function
void setSchedulerName(const char* name);

// Declare the get_user_input function
char* get_user_input(const char* prompt_message);

// Declare the append_to_output function
void append_to_output(const char *text);

// Define the InitializeProcessData struct
typedef struct {
    GtkWidget *process_dropdown;
    GtkWidget *arrival_entry;
    GtkWidget *quantum_entry;
} InitializeProcessData;

#endif // DASHBOARD_H
