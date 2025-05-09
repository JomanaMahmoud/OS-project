#include <gtk/gtk.h>
#include <glib.h>
#include "dashboard.h"
#include "memory_viewer_tab.h"
#include "mutex_tab.h"
#include "main.h" // Include main.h for type definitions

// GTK widgets
static GtkWidget *window;
static GtkWidget *clock_label;
static GtkWidget *process_count_label;
static GtkWidget *pending_count_label;
static GtkWidget *ready_queue_view;
static GtkWidget *blocked_queue_view;
static GtkWidget *running_process_view;
static GtkWidget *pending_processes_view;
static GtkTextBuffer *ready_queue_buffer;
static GtkTextBuffer *blocked_queue_buffer;
static GtkTextBuffer *running_process_buffer;
static GtkTextBuffer *pending_processes_buffer;
static GtkWidget *scheduler_name_label;
static GtkWidget *processes_list_view; // New widget for processes list
static GtkTextBuffer *processes_list_buffer;
GtkWidget *next_step_button;
GtkWidget *auto_button;
GtkWidget *stop_button; // New Stop button
GtkWidget *reset_button; // New Reset button
guint auto_timer_id = 0;


extern GtkTextBuffer *memory_viewer_buffer; // Declare the memory viewer buffer
extern GtkGrid *mutex_grid;

// Declare a global text buffer for the output
static GtkTextBuffer *output_buffer;

// Declare a global text buffer for user output
static GtkTextBuffer *user_output_buffer;

// Function declarations
void setInputHandler(void (*handler)(const char *input));
void printAllProcesses();
void passTurn();
void printReadyQueue();
void printQueue(Queue* q);
void printPendingProcesses();
void on_step_by_step_clicked(GtkWidget *widget, gpointer data);
void on_next_step_clicked(GtkWidget *widget, gpointer data);
gboolean auto_execute(gpointer data);
void on_auto_execution_clicked(GtkWidget *widget, gpointer data);
void on_add_process_clicked(GtkWidget *widget, gpointer data); // New function declaration
GtkWidget* create_process_and_scheduler_tab(); // New function declaration
char* get_user_input(const char* prompt_message); // New function declaration
char* frontend_input_handler(const char* prompt_message); // New function declaration
GtkWidget* create_output_section(); // New function declaration
void append_to_output(const char *text); // New function declaration
GtkWidget* create_user_output_section(); // New function declaration
void append_to_user_output(const char *text); // New function declaration
void on_stop_execution_clicked(GtkWidget *widget, gpointer data); // New function declaration
void on_reset_button_clicked(GtkWidget *widget, gpointer data); // New function declaration
void printMLFQToOutput(); // New function declaration
// Forward declarations for refresh_gui_data
void capture_ready_queue_output();
void capture_blocked_queue_output();
void capture_running_process_output();
void capture_pending_processes_output();
void update_clock_label();
void update_pending_count_label();
void refresh_memory_viewer(GtkTextBuffer *buffer); // Forward declaration for memory viewer refresh
void refresh_mutex_tab(GtkGrid *grid); // Forward declaration for mutex tab refresh

// Define the wrapper function
void frontend_input_handler_wrapper(const char *input) {
    char *result = frontend_input_handler(input);
    if (result) {
       // g_free(result); // Ensure this is only freed once
    }
}
void clearOutput() {
    printf("Output cleared.\n");
}

void clearUserOutput() {
    printf("User output cleared.\n");
}
// Function to apply dark theme to the dashboard
void apply_dark_theme(GtkWidget* widget) {
    GtkCssProvider* provider = gtk_css_provider_new();
    GtkStyleContext* context = gtk_widget_get_style_context(widget);

    // Define CSS for dark background and light text
    const char* css = "window { background-color: #2E2E2E; color:  #2E2E2E; }";

    // Load the CSS into the provider
    gtk_css_provider_load_from_data(provider, css, -1, NULL);

    // Apply the CSS to the widget's style context
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Free the CSS provider
    g_object_unref(provider);
}

void on_step_by_step_clicked(GtkWidget *widget, gpointer data) {
    gtk_widget_show(next_step_button); // Show "Next Step" button
}
void refresh_gui_data() {
    capture_ready_queue_output();
    capture_blocked_queue_output();
    capture_running_process_output();
    capture_pending_processes_output();
    update_clock_label();
    update_pending_count_label();

    // Refresh the memory viewer
    
        refresh_memory_viewer(memory_viewer_buffer);
    

    //Refresh the mutex tab
    
        refresh_mutex_tab(mutex_grid);
    
}

void update_clock_label() {
    
}

void update_pending_count_label() {
   
}


void on_next_step_clicked(GtkWidget *widget, gpointer data) {
    passTurn();
    refresh_gui_data(); // Redraw GUI (your own function)
    refresh_mutex_tab(mutex_grid);
    
}

gboolean auto_execute(gpointer data) {
    // Stop condition: no more processes
    if (readyQueue.size == 0 && !runningProcess && pendingCount == 0) {
        auto_timer_id = 0;
        return FALSE; // Stop the timer
    }
    passTurn();
    refresh_gui_data();
    return TRUE; // Continue
}

void on_auto_execution_clicked(GtkWidget *widget, gpointer data) {
    if (auto_timer_id == 0) {
        auto_timer_id = g_timeout_add(1000, auto_execute, NULL); // Call every 1 second
    }
}

// Function to stop auto-execution
void on_stop_execution_clicked(GtkWidget *widget, gpointer data) {
    if (auto_timer_id != 0) {
        g_source_remove(auto_timer_id); // Stop the timer
        auto_timer_id = 0;
        g_print("Auto-execution stopped.\n");
    }
}

// Function to reset all queues and clock cycle
void on_reset_button_clicked(GtkWidget *widget, gpointer data) {
    // Reset clock cycle
    clockCycle = 0;

    // Clear all queues
    initQueue(&readyQueue);
    initQueue(&globalBlockedQueue);
    mutex_userInput.blockedQueue->size = 0;
    mutex_userOutput.blockedQueue->size = 0;
    mutex_file.blockedQueue->size = 0;

    // Reset memory
    resetMemory();

    // Clear output and user output sections
    clearOutput();
    clearUserOutput();

    // Refresh the GUI
    refresh_gui_data();
    g_print("Simulation reset.\n");
}

// Function to get user input via a dialog
char* get_user_input(const char* prompt_message) {
    GtkWidget *dialog, *content_area, *entry;
    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
    char *user_input = NULL;

    // Create the dialog
    dialog = gtk_dialog_new_with_buttons(
        "User Input",
        GTK_WINDOW(window),
        flags,
        "_OK",
        GTK_RESPONSE_OK,
        "_Cancel",
        GTK_RESPONSE_CANCEL,
        NULL
    );

    // Add content to the dialog
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new(prompt_message);
    gtk_box_pack_start(GTK_BOX(content_area), label, FALSE, FALSE, 5);

    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(content_area), entry, FALSE, FALSE, 5);

    gtk_widget_show_all(dialog);

    // Run the dialog and get the response
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
        user_input = g_strdup(text); // Duplicate the text to return
    }

    gtk_widget_destroy(dialog);
    return user_input;
}

// Set the frontend input handler
char* frontend_input_handler(const char* prompt_message) {
    return get_user_input(prompt_message);
}

// Example usage of the input dialog in a function
void on_add_process_clicked(GtkWidget *widget, gpointer data) {
    // Get quantum value from the user
    char *quantum_text = get_user_input("Enter Quantum Value:");
    if (!quantum_text || *quantum_text == '\0') {
        g_print("Error: Quantum value is empty.\n");
        g_free(quantum_text);
        return;
    }

    int quantum = atoi(quantum_text);
    g_free(quantum_text);

    if (quantum <= 0) {
        g_print("Error: Invalid quantum value. Must be greater than 0.\n");
        return;
    }

    g_print("Quantum value set to: %d\n", quantum);

    // Prompt the user to select a process file
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Select Process File",
        GTK_WINDOW(window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT,
        NULL
    );

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        g_print("Selected process file: %s\n", filename);

        // Load the process file (placeholder for actual implementation)
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

// Example usage of the input dialog in a function
void example_usage_of_input_dialog() {
    char *input = get_user_input("Enter Quantum Value:");
    if (input) {
        g_print("User entered: %s\n", input);
        g_free(input);
    }
}

// Helper function to create styled labels
GtkWidget* create_styled_label(const char* text, const char* css_class) {
    GtkWidget *label = gtk_label_new(text);
    gtk_widget_set_name(label, css_class);
    return label;
}

// Helper function to create text views with styling
static GtkWidget* create_styled_text_view(GtkTextBuffer **buffer, const char *css_class) {
    GtkWidget *view = gtk_text_view_new();
    if (buffer) {
        *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    }
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD_CHAR);
    gtk_widget_set_name(view, css_class);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), view);

    return scroll;
}

// Function to append text to the output buffer
void append_to_output(const char *text) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(output_buffer, &end);
    gtk_text_buffer_insert(output_buffer, &end, text, -1);
}

// Function to append text to the user output buffer
void append_to_user_output(const char *text) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(user_output_buffer, &end);
    gtk_text_buffer_insert(user_output_buffer, &end, text, -1);
}

// Create the output section in the GUI
GtkWidget* create_output_section() {
    GtkWidget *frame = gtk_frame_new("Output");
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    GtkWidget *view = gtk_text_view_new();
    output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD_CHAR);

    gtk_container_add(GTK_CONTAINER(scroll), view);
    gtk_container_add(GTK_CONTAINER(frame), scroll);

    return frame;
}

// Create the user output section in the GUI
GtkWidget* create_user_output_section() {
    GtkWidget *frame = gtk_frame_new("User Output");
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    GtkWidget *view = gtk_text_view_new();
    user_output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD_CHAR);

    gtk_container_add(GTK_CONTAINER(scroll), view);
    gtk_container_add(GTK_CONTAINER(frame), scroll);

    return frame;
}

// Function to capture ready queue information
void capture_ready_queue_output() {
    FILE *original_stdout = stdout;
    FILE *temp_file = fopen("ready_queue_output.txt", "w");
    if (!temp_file) {
        perror("Could not open ready_queue_output.txt");
        return;
    }

    fflush(stdout);
    stdout = temp_file;
    
    // Print ready queue info
    printReadyQueue();
    
    fflush(stdout);
    stdout = original_stdout;
    fclose(temp_file);
    
    // Load and set text
    FILE *input = fopen("ready_queue_output.txt", "r");
    if (!input) {
        perror("Could not read ready_queue_output.txt");
        return;
    }

    fseek(input, 0, SEEK_END);
    long len = ftell(input);
    fseek(input, 0, SEEK_SET);

    char *contents = malloc(len + 1);
    fread(contents, 1, len, input);
    contents[len] = '\0';
    fclose(input);

    gtk_text_buffer_set_text(ready_queue_buffer, contents, -1);
    free(contents);
    remove("ready_queue_output.txt");
}

// Function to capture blocked queue information
void capture_blocked_queue_output() {
    FILE *original_stdout = stdout;
    FILE *temp_file = fopen("blocked_queue_output.txt", "w");
    if (!temp_file) {
        perror("Could not open blocked_queue_output.txt");
        return;
    }

    fflush(stdout);
    stdout = temp_file;
    
    // Print blocked queue info
    printQueue(&globalBlockedQueue);
    
    fflush(stdout);
    stdout = original_stdout;
    fclose(temp_file);
    
    // Load and set text
    FILE *input = fopen("blocked_queue_output.txt", "r");
    if (!input) {
        perror("Could not read blocked_queue_output.txt");
        return;
    }

    fseek(input, 0, SEEK_END);
    long len = ftell(input);
    fseek(input, 0, SEEK_SET);

    char *contents = malloc(len + 1);
    fread(contents, 1, len, input);
    contents[len] = '\0';
    fclose(input);

    gtk_text_buffer_set_text(blocked_queue_buffer, contents, -1);
    free(contents);
    remove("blocked_queue_output.txt");
}

// Function to capture running process information
void capture_running_process_output() {
    FILE *original_stdout = stdout;
    FILE *temp_file = fopen("running_process_output.txt", "w");
    if (!temp_file) {
        perror("Could not open running_process_output.txt");
        return;
    }

    fflush(stdout);
    stdout = temp_file;
    
    // Print running process info
    printf("🟢 Running Process:\n");
    if (runningProcess == NULL) {
        printf(" (none)\n");
    } else {
        printf(" PID: %d\n", runningProcess->pid);
        printf(" Lower: [%d]\n", runningProcess->memoryLowerBound);
        printf(" Upper: [%d]\n", runningProcess->memoryUpperBound);
        printf("Priority: %d\n", runningProcess->priority);
        printf(" Program Counter: %d\n", runningProcess->programCounter);
    }
    
    fflush(stdout);
    stdout = original_stdout;
    fclose(temp_file);
    
    // Load and set text
    FILE *input = fopen("running_process_output.txt", "r");
    if (!input) {
        perror("Could not read running_process_output.txt");
        return;
    }

    fseek(input, 0, SEEK_END);
    long len = ftell(input);
    fseek(input, 0, SEEK_SET);

    char *contents = malloc(len + 1);
    fread(contents, 1, len, input);
    contents[len] = '\0';
    fclose(input);

    gtk_text_buffer_set_text(running_process_buffer, contents, -1);
    free(contents);
    remove("running_process_output.txt");
}

// Helper to capture stdout output of printPendingProcesses
void capture_pending_processes_output() {
    FILE *original_stdout = stdout;
    FILE *temp_file = fopen("pending_processes_output.txt", "w");
    if (!temp_file) {
        perror("Could not open pending_processes_output.txt");
        return;
    }

    // Redirect stdout
    fflush(stdout);
    stdout = temp_file;

    // Call the existing function
    printPendingProcesses();

    // Restore stdout
    fflush(stdout);
    stdout = original_stdout;
    fclose(temp_file);

    // Read the file contents
    FILE *input = fopen("pending_processes_output.txt", "r");
    if (!input) {
        perror("Could not read pending_processes_output.txt");
        return;
    }

    fseek(input, 0, SEEK_END);
    long len = ftell(input);
    fseek(input, 0, SEEK_SET);

    char *contents = malloc(len + 1);
    fread(contents, 1, len, input);
    contents[len] = '\0';
    fclose(input);

    // Set text in TextView buffer
    gtk_text_buffer_set_text(pending_processes_buffer, contents, -1);
    free(contents);
    remove("pending_processes_output.txt");
}

// Placeholder function for process list - to be implemented
void update_processes_list() {
        FILE *original_stdout = stdout;
        FILE *temp_file = fopen("all_processes_output.txt", "w");
        if (!temp_file) {
            perror("Could not open all_processes_output.txt");
            return;
        }
    
        fflush(stdout);
        stdout = temp_file;
    
        // Call the backend function
        printAllProcesses();
    
        fflush(stdout);
        stdout = original_stdout;
        fclose(temp_file);
    
        FILE *input = fopen("all_processes_output.txt", "r");
        if (!input) {
            perror("Could not read all_processes_output.txt");
            return;
        }
    
        fseek(input, 0, SEEK_END);
        long len = ftell(input);
        fseek(input, 0, SEEK_SET);
    
        char *contents = malloc(len + 1);
        fread(contents, 1, len, input);
        contents[len] = '\0';
        fclose(input);
    
        gtk_text_buffer_set_text(processes_list_buffer, contents, -1);
        free(contents);
        remove("all_processes_output.txt");
    }


// Update system statistics
static void update_system_stats() {
    char buf[128];
    
    // Update clock cycle display with elegant formatting
    snprintf(buf, sizeof(buf), "⏱ Clock Cycle: %d", clockCycle);
    gtk_label_set_text(GTK_LABEL(clock_label), buf);
    
    // Update process count
    
    // Update pending count
    snprintf(buf, sizeof(buf), "⏳ Pending Count: %d", pendingCount);
    gtk_label_set_text(GTK_LABEL(pending_count_label), buf);

    snprintf(buf, sizeof(buf), "🔄 Scheduler: %s", schedulerName);
    gtk_label_set_text(GTK_LABEL(scheduler_name_label), buf);
}

// Periodic GUI update
static gboolean on_timeout(gpointer user_data) {
    // Update system statistics
    update_system_stats();
    
    // Update queue and process information
    capture_ready_queue_output();
    capture_blocked_queue_output();
    capture_running_process_output();
    capture_pending_processes_output();
    update_processes_list(); // Update the processes list (placeholder)

    // Refresh the mutex tab
    

    return TRUE; // Continue the timer
}

// Function to create the header section with buttons
static GtkWidget* create_header_section() {
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(header_box, 20);
    gtk_widget_set_margin_end(header_box, 20);
    gtk_widget_set_margin_top(header_box, 20);

    // Create title with larger font
    GtkWidget *title_label = create_styled_label("Main Dashboard", "title-label");
    gtk_box_pack_start(GTK_BOX(header_box), title_label, FALSE, FALSE, 10);

    // Add simulation buttons below the title
    GtkWidget *buttons_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(buttons_box, GTK_ALIGN_CENTER);

    // Next Step button
    next_step_button = gtk_button_new_with_label("Next Step");
    g_signal_connect(next_step_button, "clicked", G_CALLBACK(on_next_step_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(buttons_box), next_step_button, FALSE, FALSE, 0);

    // Auto Execute button
    auto_button = gtk_button_new_with_label("Auto Execute");
    g_signal_connect(auto_button, "clicked", G_CALLBACK(on_auto_execution_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(buttons_box), auto_button, FALSE, FALSE, 0);

    // Stop button
    stop_button = gtk_button_new_with_label("Stop");
    g_signal_connect(stop_button, "clicked", G_CALLBACK(on_stop_execution_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(buttons_box), stop_button, FALSE, FALSE, 0);

    // Reset button
    reset_button = gtk_button_new_with_label("Reset");
    g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(buttons_box), reset_button, FALSE, FALSE, 0);

    // Add the buttons box to the header
    gtk_box_pack_start(GTK_BOX(header_box), buttons_box, FALSE, FALSE, 10);

    // Create statistics box (horizontal)
    GtkWidget *stats_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(stats_box, GTK_ALIGN_CENTER);

    // Create system info labels
    clock_label = create_styled_label("⏱ Clock Cycle: 0", "data-label");
    process_count_label = create_styled_label("💻 Total Processes: 3", "data-label");
    pending_count_label = create_styled_label("⏳ Pending Count: 0", "data-label");
    scheduler_name_label = create_styled_label("🔄 Scheduler:", "data-label");

    // Add labels to stats box
    gtk_box_pack_start(GTK_BOX(stats_box), clock_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(stats_box), process_count_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(stats_box), pending_count_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(stats_box), scheduler_name_label, FALSE, FALSE, 0);

    // Add stats box to header
    gtk_box_pack_start(GTK_BOX(header_box), stats_box, FALSE, FALSE, 0);

    return header_box;
}

// Create left pane for queues
static GtkWidget* create_queues_section() {
    // Main container for all queue displays
    GtkWidget *queues_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(queues_box, 10);
    gtk_widget_set_margin_end(queues_box, 5);
    gtk_widget_set_margin_top(queues_box, 10);
    gtk_widget_set_margin_bottom(queues_box, 10);
    
    // Ready Queue Section
    GtkWidget *ready_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *ready_label = create_styled_label("Ready Queue", "queue-title-label");
    ready_queue_view = create_styled_text_view(&ready_queue_buffer, "ready-queue");
    gtk_box_pack_start(GTK_BOX(ready_box), ready_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ready_box), ready_queue_view, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(queues_box), ready_box, TRUE, TRUE, 0);
    
    // Blocked Queue Section
    GtkWidget *blocked_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *blocked_label = create_styled_label("Blocked Queue", "queue-title-label");
    blocked_queue_view = create_styled_text_view(&blocked_queue_buffer, "blocked-queue");
    gtk_box_pack_start(GTK_BOX(blocked_box), blocked_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(blocked_box), blocked_queue_view, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(queues_box), blocked_box, TRUE, TRUE, 0);
    
    // Running Process Section
    GtkWidget *running_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *running_label = create_styled_label("Running Process", "queue-title-label");
    running_process_view = create_styled_text_view(&running_process_buffer, "running-process");
    gtk_box_pack_start(GTK_BOX(running_box), running_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(running_box), running_process_view, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(queues_box), running_box, TRUE, TRUE, 0);
    
    // Pending Processes Section
    GtkWidget *pending_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *pending_label = create_styled_label("Pending Processes", "queue-title-label");
    pending_processes_view = create_styled_text_view(&pending_processes_buffer, "pending-processes");
    gtk_box_pack_start(GTK_BOX(pending_box), pending_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pending_box), pending_processes_view, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(queues_box), pending_box, TRUE, TRUE, 0);
    
    return queues_box;
}

// Create right pane for process list
static GtkWidget* create_processes_list_section() {
    GtkWidget *processes_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_start(processes_box, 5);
    gtk_widget_set_margin_end(processes_box, 10);
    gtk_widget_set_margin_top(processes_box, 10);
    gtk_widget_set_margin_bottom(processes_box, 10);
    
    GtkWidget *processes_label = create_styled_label("Process List", "queue-title-label");
    gtk_box_pack_start(GTK_BOX(processes_box), processes_label, FALSE, FALSE, 0);
    
    // Create text view for processes list (fixed version)
    processes_list_view = create_styled_text_view(&processes_list_buffer, "processes-list");
    gtk_box_pack_start(GTK_BOX(processes_box), processes_list_view, TRUE, TRUE, 0);
    
    return processes_box;
}

void create_gui(void) {
    // Set the frontend input handler
    setInputHandler(frontend_input_handler_wrapper);

    // Initialize GTK
    gtk_init(NULL, NULL);

    // Create main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "OS Task Dashboard");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 800);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Apply dark theme to the window
    apply_dark_theme(window);

    // Create main vertical box
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    // Create a GtkNotebook for tabs
    GtkWidget *notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(main_box), notebook, TRUE, TRUE, 0);

    // Tab 1: Process and Scheduler
    GtkWidget *process_and_scheduler_tab = create_process_and_scheduler_tab();
    GtkWidget *process_and_scheduler_label = gtk_label_new("Process and Scheduler");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), process_and_scheduler_tab, process_and_scheduler_label);

    // Tab 2: Main Dashboard
    GtkWidget *current_layout_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Add header section (includes buttons under the title)
    gtk_box_pack_start(GTK_BOX(current_layout_tab), create_header_section(), FALSE, FALSE, 0);

    // Add separator below header
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_start(separator, 10);
    gtk_widget_set_margin_end(separator, 10);
    gtk_box_pack_start(GTK_BOX(current_layout_tab), separator, FALSE, FALSE, 10);

    // Create horizontal box for split view (queues left, processes right)
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(current_layout_tab), content_box, TRUE, TRUE, 0);

    // Create paned container to allow resizing the split
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(content_box), paned, TRUE, TRUE, 0);

    // Add left section (queues)
    gtk_paned_pack1(GTK_PANED(paned), create_queues_section(), TRUE, FALSE);

    // Add right section (processes list)
    gtk_paned_pack2(GTK_PANED(paned), create_processes_list_section(), TRUE, FALSE);

    // Set initial position for the paned split (40% for queues, 60% for processes)
    gtk_paned_set_position(GTK_PANED(paned), 400);

    // Add the current layout tab to the notebook
    GtkWidget *current_layout_label = gtk_label_new("Main Dashboard");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), current_layout_tab, current_layout_label);

    // Tab 3: Memory Viewer
    GtkWidget *memory_viewer_tab = create_memory_viewer_tab(&memory_viewer_buffer);
    GtkWidget *memory_viewer_label = gtk_label_new("Memory Viewer");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), memory_viewer_tab, memory_viewer_label);

    // Tab 4: Mutex
    GtkWidget *mutex_tab = create_mutex_tab(&mutex_grid);
    GtkWidget *mutex_label = gtk_label_new("Mutex");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), mutex_tab, mutex_label);

    // Tab 5: Output Section
    GtkWidget *output_section_tab = create_output_section();
    GtkWidget *output_section_label = gtk_label_new("Output");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), output_section_tab, output_section_label);

    // Tab 6: User Output Section
    GtkWidget *user_output_section_tab = create_user_output_section();
    GtkWidget *user_output_section_label = gtk_label_new("User Output");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), user_output_section_tab, user_output_section_label);

    // Load CSS styling
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkScreen *screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(screen,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    GError *error = NULL;
    if (!gtk_css_provider_load_from_path(provider, "style.css", &error)) {
        g_printerr("Error loading CSS file: %s\n", error->message);
        g_clear_error(&error);
    }

    // Initial update and start periodic updates
    on_timeout(NULL);
    g_timeout_add_seconds(1, on_timeout, NULL);

    // Show all widgets
    gtk_widget_show_all(window);

    // Run GTK main loop
    gtk_main();
}

void printMLFQToOutput() {
    char buffer[512];

    // Print Level L1
    snprintf(buffer, sizeof(buffer), "\n🟩 Level L1:\n");
    append_to_output(buffer);
    if (L1->size == 0) {
        append_to_output("  (empty)\n");
    } else {
        int idx = L1->front;
        for (int i = 0; i < L1->size; i++) {
            PCB *pcb = L1->items[idx];
            snprintf(buffer, sizeof(buffer), "  → PID: %d | Priority: %d | Memory: [%d-%d]\n",
                     pcb->pid, pcb->priority, pcb->memoryLowerBound, pcb->memoryUpperBound);
            append_to_output(buffer);
            idx = (idx + 1) % MAX_QUEUE_SIZE;
        }
    }

    // Print Level L2
    snprintf(buffer, sizeof(buffer), "\n🟨 Level L2:\n");
    append_to_output(buffer);
    if (L2->size == 0) {
        append_to_output("  (empty)\n");
    } else {
        int idx = L2->front;
        for (int i = 0; i < L2->size; i++) {
            PCB *pcb = L2->items[idx];
            snprintf(buffer, sizeof(buffer), "  → PID: %d | Priority: %d | Memory: [%d-%d]\n",
                     pcb->pid, pcb->priority, pcb->memoryLowerBound, pcb->memoryUpperBound);
            append_to_output(buffer);
            idx = (idx + 1) % MAX_QUEUE_SIZE;
        }
    }

    // Print Level L3
    snprintf(buffer, sizeof(buffer), "\n🟧 Level L3:\n");
    append_to_output(buffer);
    if (L3->size == 0) {
        append_to_output("  (empty)\n");
    } else {
        int idx = L3->front;
        for (int i = 0; i < L3->size; i++) {
            PCB *pcb = L3->items[idx];
            snprintf(buffer, sizeof(buffer), "  → PID: %d | Priority: %d | Memory: [%d-%d]\n",
                     pcb->pid, pcb->priority, pcb->memoryLowerBound, pcb->memoryUpperBound);
            append_to_output(buffer);
            idx = (idx + 1) % MAX_QUEUE_SIZE;
        }
    }

    // Print Level L4
    snprintf(buffer, sizeof(buffer), "\n🟥 Level L4:\n");
    append_to_output(buffer);
    if (L4->size == 0) {
        append_to_output("  (empty)\n");
    } else {
        int idx = L4->front;
        for (int i = 0; i < L4->size; i++) {
            PCB *pcb = L4->items[idx];
            snprintf(buffer, sizeof(buffer), "  → PID: %d | Priority: %d | Memory: [%d-%d]\n",
                     pcb->pid, pcb->priority, pcb->memoryLowerBound, pcb->memoryUpperBound);
            append_to_output(buffer);
            idx = (idx + 1) % MAX_QUEUE_SIZE;
        }
    }
}