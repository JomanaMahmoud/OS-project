#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include "main.h" // Include the header file where Mutex and Queue are defined

GtkGrid *mutex_grid = NULL; // Define the global mutex grid

// Function to display the contents of a queue
void display_queue(GtkWidget *parent_box, const char *queue_name, Queue *queue) {
    // Add a label for the queue name
    GtkWidget *queue_label = gtk_label_new(queue_name);
    gtk_box_pack_start(GTK_BOX(parent_box), queue_label, FALSE, FALSE, 5);

    // Check if the queue is NULL
    if (!queue) {
        GtkWidget *error_label = gtk_label_new("  (Queue is NULL)");
        gtk_box_pack_start(GTK_BOX(parent_box), error_label, FALSE, FALSE, 5);
        return;
    }

    // Check if the queue is empty
    if (queue->size == 0) {
        GtkWidget *empty_label = gtk_label_new("  (Queue is empty)");
        gtk_box_pack_start(GTK_BOX(parent_box), empty_label, FALSE, FALSE, 5);
        return;
    }

    // Iterate through the queue and display each PCB
    int idx = queue->front;
    for (int i = 0; i < queue->size; i++) {
        PCB *pcb = queue->items[idx];
        if (pcb) {
            char pcb_info[128];
            snprintf(pcb_info, sizeof(pcb_info), "  → PID: %d | Priority: %d", pcb->pid, pcb->priority);
            GtkWidget *pcb_label = gtk_label_new(pcb_info);
            gtk_box_pack_start(GTK_BOX(parent_box), pcb_label, FALSE, FALSE, 5);
        } else {
            GtkWidget *error_label = gtk_label_new("  (Invalid PCB in queue)");
            gtk_box_pack_start(GTK_BOX(parent_box), error_label, FALSE, FALSE, 5);
        }
        idx = (idx + 1) % MAX_QUEUE_SIZE; // Circular queue navigation
    }
}

// Function to create the Mutex Status section
GtkWidget* create_mutex_status_section() {
    GtkWidget *frame = gtk_frame_new("Mutex Status");
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    // Mutex: userInput
    char status[256];
    snprintf(status, sizeof(status), "Mutex: userInput\n  Status: %s\n",
             mutex_userInput.isAvailable ? "Available" : "Locked");
    GtkWidget *label_userInput = gtk_label_new(status);
    gtk_box_pack_start(GTK_BOX(box), label_userInput, FALSE, FALSE, 0);

    // Mutex: userOutput
    snprintf(status, sizeof(status), "Mutex: userOutput\n  Status: %s\n",
             mutex_userOutput.isAvailable ? "Available" : "Locked");
    GtkWidget *label_userOutput = gtk_label_new(status);
    gtk_box_pack_start(GTK_BOX(box), label_userOutput, FALSE, FALSE, 0);

    // Mutex: file
    snprintf(status, sizeof(status), "Mutex: file\n  Status: %s\n",
             mutex_file.isAvailable ? "Available" : "Locked");
    GtkWidget *label_file = gtk_label_new(status);
    gtk_box_pack_start(GTK_BOX(box), label_file, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(frame), box);
    return frame;
}

// Function to create the Blocked Queue section
GtkWidget* create_blocked_queue_section() {
    GtkWidget *frame = gtk_frame_new("Blocked Queues");
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    // Display queues for userInput
    display_queue(box, "Blocked Queue: userInput", mutex_userInput.blockedQueue);

    // Display queues for userOutput
    display_queue(box, "Blocked Queue: userOutput", mutex_userOutput.blockedQueue);

    // Display queues for file
    display_queue(box, "Blocked Queue: file", mutex_file.blockedQueue);

    gtk_container_add(GTK_CONTAINER(frame), box);
    return frame;
}

// Function to create the Mutex Tab
GtkWidget* create_mutex_tab(GtkGrid **out_grid) {
    GtkWidget *mutex_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(mutex_box, 10);
    gtk_widget_set_margin_end(mutex_box, 10);
    gtk_widget_set_margin_top(mutex_box, 10);
    gtk_widget_set_margin_bottom(mutex_box, 10);

    // Create a grid for the mutex tab
    GtkWidget *grid = gtk_grid_new(); // Use GtkWidget * for the grid
    if (out_grid) {
        *out_grid = GTK_GRID(grid); // Cast to GtkGrid * when assigning to out_grid
    }
    gtk_box_pack_start(GTK_BOX(mutex_box), GTK_WIDGET(grid), TRUE, TRUE, 0);

    // Add Mutex Status Section
    GtkWidget *mutex_status_section = create_mutex_status_section();
    gtk_grid_attach(GTK_GRID(grid), mutex_status_section, 0, 0, 1, 1);

    // Add Blocked Queue Section
    GtkWidget *blocked_queue_section = create_blocked_queue_section();
    gtk_grid_attach(GTK_GRID(grid), blocked_queue_section, 0, 1, 1, 1);

    return mutex_box;
}

// Function to refresh the Mutex Tab
void refresh_mutex_tab(GtkGrid *mutex_grid) {
    if (!mutex_grid) {
        return;
    }

    // Clear the grid
    GList *children = gtk_container_get_children(GTK_CONTAINER(mutex_grid));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    // Add updated sections
    GtkWidget *mutex_status_section = create_mutex_status_section();
    gtk_grid_attach(GTK_GRID(mutex_grid), mutex_status_section, 0, 0, 1, 1);

    GtkWidget *blocked_queue_section = create_blocked_queue_section();
    gtk_grid_attach(GTK_GRID(mutex_grid), blocked_queue_section, 0, 1, 1, 1);

    gtk_widget_show_all(GTK_WIDGET(mutex_grid));
}