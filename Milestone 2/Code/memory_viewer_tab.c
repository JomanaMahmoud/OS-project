#include <gtk/gtk.h>
#include <string.h>
#include "memory_viewer_tab.h"

// Declare the MemoryCell structure and memory array from main.c
typedef struct {
    char name[32];        // "Instruction", "PID", etc.
    char data[64];        // The actual instruction or value
    int isInstruction;    // 1 if it's an instruction, 0 otherwise
} MemoryCell;

extern MemoryCell memory[60]; // Declare the memory array

GtkTextBuffer *memory_viewer_buffer = NULL; // Define the memory viewer buffer globally

// Function to refresh the memory viewer
void refresh_memory_viewer(GtkTextBuffer *buffer) {
    char memory_contents[4096] = ""; // Buffer to hold memory contents
    snprintf(memory_contents, sizeof(memory_contents), "\n📦 Memory Contents:\n");

    for (int i = 0; i < 60; i++) { // Iterate through all 60 memory cells
        char line[128];
        const char *nameToPrint = strlen(memory[i].name) > 0 ? memory[i].name : "<empty>";
        const char *dataToPrint = strlen(memory[i].data) > 0 ? memory[i].data : "<empty>";
        snprintf(line, sizeof(line), "[%02d] name: %-12s | data: %-24s\n",
                 i, nameToPrint, dataToPrint); // Removed the isInstruction part
        strncat(memory_contents, line, sizeof(memory_contents) - strlen(memory_contents) - 1);
    }

    gtk_text_buffer_set_text(buffer, memory_contents, -1);
}

// Function to create the "Memory Viewer" tab
GtkWidget* create_memory_viewer_tab() {
    // Create a vertical box to hold the memory viewer components
    GtkWidget *memory_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(memory_box, 10);
    gtk_widget_set_margin_end(memory_box, 10);
    gtk_widget_set_margin_top(memory_box, 10);
    gtk_widget_set_margin_bottom(memory_box, 10);

    // Add a label for the tab title
    GtkWidget *title_label = gtk_label_new("Memory Viewer");
    gtk_box_pack_start(GTK_BOX(memory_box), title_label, FALSE, FALSE, 10);

    // Create a text view to display memory cells
    GtkWidget *memory_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(memory_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(memory_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(memory_view), GTK_WRAP_WORD);
    gtk_widget_set_name(memory_view, "memory-view");

    // Create a text buffer for the memory view
    memory_viewer_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(memory_view));

    // Populate the memory view with initial data
    refresh_memory_viewer(memory_viewer_buffer);

    // Add the text view to a scrolled window
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), memory_view);
    gtk_box_pack_start(GTK_BOX(memory_box), scrolled_window, TRUE, TRUE, 10);

    return memory_box;
}
void resetMemory() {
    printf("Memory reset.\n");
}