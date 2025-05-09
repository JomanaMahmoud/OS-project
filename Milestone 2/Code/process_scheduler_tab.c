#include <gtk/gtk.h>
#include "dashboard.h"

// Declare the callback functions
void on_set_scheduler_clicked(GtkWidget *widget, gpointer scheduler_dropdown);
void on_scheduler_changed(GtkComboBoxText *scheduler_dropdown, gpointer quantum_entry);
void on_initialize_process_clicked(GtkWidget *widget, gpointer user_data);

// Function to create the "Process and Scheduler" tab
GtkWidget* create_process_and_scheduler_tab() {
    GtkWidget *tab_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(tab_box, 10);
    gtk_widget_set_margin_end(tab_box, 10);
    gtk_widget_set_margin_top(tab_box, 10);
    gtk_widget_set_margin_bottom(tab_box, 10);

    // Add a label for the tab title
    GtkWidget *title_label = create_styled_label("Process and Scheduler", "title-label");
    gtk_box_pack_start(GTK_BOX(tab_box), title_label, FALSE, FALSE, 10);

    // Add a dropdown for selecting the scheduling algorithm
    GtkWidget *scheduler_label = create_styled_label("Select Scheduling Algorithm:", "data-label");
    GtkWidget *scheduler_dropdown = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scheduler_dropdown), "First Come First Serve");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scheduler_dropdown), "Round Robin");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scheduler_dropdown), "Multilevel Feedback Queue");
    gtk_box_pack_start(GTK_BOX(tab_box), scheduler_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(tab_box), scheduler_dropdown, FALSE, FALSE, 5);

    // Add an input field for adjusting the quantum (hidden by default)
    GtkWidget *quantum_label = create_styled_label("Adjust Quantum:", "data-label");
    GtkWidget *quantum_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(tab_box), quantum_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(tab_box), quantum_entry, FALSE, FALSE, 5);
    gtk_widget_hide(quantum_label);
    gtk_widget_hide(quantum_entry);

    // Show or hide quantum input based on selected scheduling algorithm
    g_signal_connect(scheduler_dropdown, "changed", G_CALLBACK(on_scheduler_changed), quantum_entry);

    // Add a dropdown for selecting the process file
    GtkWidget *process_label = create_styled_label("Select Process File:", "data-label");
    GtkWidget *process_dropdown = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(process_dropdown), "Program_1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(process_dropdown), "Program_2");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(process_dropdown), "Program_3");
    gtk_box_pack_start(GTK_BOX(tab_box), process_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(tab_box), process_dropdown, FALSE, FALSE, 5);

    // Add an input field for the arrival time
    GtkWidget *arrival_label = create_styled_label("Arrival Time:", "data-label");
    GtkWidget *arrival_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(tab_box), arrival_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(tab_box), arrival_entry, FALSE, FALSE, 5);

    // Add a button to initialize the process
    GtkWidget *initialize_button = gtk_button_new_with_label("Initialize Process");
    InitializeProcessData *data = g_new0(InitializeProcessData, 1);
    data->process_dropdown = process_dropdown;
    data->arrival_entry = arrival_entry;
    g_signal_connect(initialize_button, "clicked", G_CALLBACK(on_initialize_process_clicked), data);
    gtk_box_pack_start(GTK_BOX(tab_box), initialize_button, FALSE, FALSE, 10);

    // Add a button to set the scheduler
    GtkWidget *set_scheduler_button = gtk_button_new_with_label("Set Scheduler");
    g_signal_connect(set_scheduler_button, "clicked", G_CALLBACK(on_set_scheduler_clicked), scheduler_dropdown);
    gtk_box_pack_start(GTK_BOX(tab_box), set_scheduler_button, FALSE, FALSE, 10);

    return tab_box;
}

// Callback to show/hide quantum input based on selected scheduling algorithm
void on_scheduler_changed(GtkComboBoxText *scheduler_dropdown, gpointer quantum_entry) {
    const char *selected_algorithm = gtk_combo_box_text_get_active_text(scheduler_dropdown);
    if (g_strcmp0(selected_algorithm, "Round Robin") == 0) {
        gtk_widget_show(GTK_WIDGET(quantum_entry));
    } else {
        gtk_widget_hide(GTK_WIDGET(quantum_entry));
    }
}

// Callback to set the scheduler
void on_set_scheduler_clicked(GtkWidget *widget, gpointer scheduler_dropdown) {
    const char *selected_algorithm = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(scheduler_dropdown));
    if (!selected_algorithm) {
        g_print("Error: No scheduling algorithm selected.\n");
        return;
    }

    // Map user-friendly names to backend scheduler names
    if (g_strcmp0(selected_algorithm, "First Come First Serve") == 0) {
        setSchedulerName("firstComeFirstServe");
    } else if (g_strcmp0(selected_algorithm, "Round Robin") == 0) {
        setSchedulerName("roundRobin");
    } else if (g_strcmp0(selected_algorithm, "Multilevel Feedback Queue") == 0) {
        setSchedulerName("mlfq");
    } else {
        g_print("Error: Unknown scheduling algorithm selected.\n");
        return;
    }

    g_print("Scheduler set to: %s\n", selected_algorithm);
}

// Callback to initialize the process
void on_initialize_process_clicked(GtkWidget *widget, gpointer user_data) {
    InitializeProcessData *data = (InitializeProcessData *)user_data;
    GtkComboBoxText *process_dropdown = GTK_COMBO_BOX_TEXT(data->process_dropdown);
    GtkEntry *arrival_entry = GTK_ENTRY(data->arrival_entry);

    // Get selected process
    const char *selected_process = gtk_combo_box_text_get_active_text(process_dropdown);
    if (!selected_process) {
        g_print("Error: No process selected.\n");
        return;
    }

    // Get arrival time
    const char *arrival_text = gtk_entry_get_text(arrival_entry);
    if (!arrival_text || *arrival_text == '\0') {
        g_print("Error: Arrival time is empty.\n");
        return;
    }
    int arrival_time = atoi(arrival_text);

    // Call the backend function
    if (g_strcmp0(selected_process, "Program_1") == 0) {
        initializeProcess("Program_1.txt", 1, arrival_time);
    } else if (g_strcmp0(selected_process, "Program_2") == 0) {
        initializeProcess("Program_2.txt", 1, arrival_time);
    } else if (g_strcmp0(selected_process, "Program_3") == 0) {
        initializeProcess("Program_3.txt", 1, arrival_time);
    }

    g_print("Initialized %s with arrival time %d.\n", selected_process, arrival_time);
}