#include <gtk/gtk.h>
#include <glib.h>
#include "dashboard.h"

// From main.c:
typedef enum {New, READY, RUNNING, BLOCKED } ProcessState;
typedef struct PCB {
    int pid;                    // Process ID
    ProcessState state;         // Current process state
    int priority;               // Priority level
    int programCounter;         // Next instruction to execute
    int memoryLowerBound;       // Start of memory allocation
    int memoryUpperBound;       // End of memory allocation         
    } PCB;

typedef struct {
    PCB* items[3];
    int front;
    int rear;
    int size;
} Queue;

extern int clockCycle;
extern int pendingCount;
extern Queue readyQueue;
extern PCB* runningProcess;
extern struct Queue globalBlockedQueue;

typedef struct {
    int pid;
    int priority;
    int arrivalTime;
    char filename[256];
    int isCreated;
} ProcessRegistration;
extern ProcessRegistration pendingProcesses[3];

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

// Function declarations
void printReadyQueue();
void printBlockedQueue(struct Queue* q);
void printPendingProcesses();

// Helper function to create styled labels
static GtkWidget* create_styled_label(const char* text, const char* css_class) {
    GtkWidget *label = gtk_label_new(text);
    gtk_widget_set_name(label, css_class);
    return label;
}

// Helper function to create text views with styling
static GtkWidget* create_styled_text_view(GtkTextBuffer **buffer, const char* css_class) {
    GtkWidget *view = gtk_text_view_new();
    *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD_CHAR);
    gtk_widget_set_name(view, css_class);
    
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), 
                                  GTK_POLICY_AUTOMATIC, 
                                  GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), view);
    
    return scroll;
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
    printBlockedQueue(&globalBlockedQueue);
    
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
        printf(" Memory Range: [%d-%d]\n", runningProcess->memoryLowerBound, runningProcess->memoryUpperBound);
        printf(" Priority: %d\n", runningProcess->priority);
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

// Update system statistics
static void update_system_stats() {
    char buf[128];
    
    // Update clock cycle display with elegant formatting
    snprintf(buf, sizeof(buf), "⏱️ Clock Cycle: %d", clockCycle);
    gtk_label_set_text(GTK_LABEL(clock_label), buf);
    
    // Update process count
    gtk_label_set_text(GTK_LABEL(process_count_label), "💻 Total Processes: 3");
    
    // Update pending count
    snprintf(buf, sizeof(buf), "⏳ Pending Count: %d", pendingCount);
    gtk_label_set_text(GTK_LABEL(pending_count_label), buf);
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

    return TRUE; // Continue the timer
}

// Create a header bar with system info
static GtkWidget* create_header_section() {
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(header_box, 20);
    gtk_widget_set_margin_end(header_box, 20);
    gtk_widget_set_margin_top(header_box, 20);
    
    // Create title with larger font
    GtkWidget *title_label = create_styled_label("Main Dashboard", "title-label");
    gtk_box_pack_start(GTK_BOX(header_box), title_label, FALSE, FALSE, 10);
    
    // Create statistics box (horizontal)
    GtkWidget *stats_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(stats_box, GTK_ALIGN_CENTER);
    
    // Create system info labels
    clock_label = create_styled_label("⏱️ Clock Cycle: 0", "data-label");
    process_count_label = create_styled_label("💻 Total Processes: 3", "data-label");
    pending_count_label = create_styled_label("⏳ Pending Count: 0", "data-label");
    
    // Add labels to stats box
    gtk_box_pack_start(GTK_BOX(stats_box), clock_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(stats_box), process_count_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(stats_box), pending_count_label, FALSE, FALSE, 0);
    
    // Add stats box to header
    gtk_box_pack_start(GTK_BOX(header_box), stats_box, FALSE, FALSE, 0);
    
    return header_box;
}

// Create queues section
static GtkWidget* create_queues_section() {
    // Main container
    GtkWidget *queues_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(queues_box, 20);
    gtk_widget_set_margin_end(queues_box, 20);
    gtk_widget_set_margin_top(queues_box, 10);
    
    // Horizontal box for ready and blocked queues
    GtkWidget *queues_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    // Ready Queue Section
    GtkWidget *ready_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *ready_label = create_styled_label("Ready Queue", "queue-title-label");
    ready_queue_view = create_styled_text_view(&ready_queue_buffer, "ready-queue");
    gtk_box_pack_start(GTK_BOX(ready_box), ready_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ready_box), ready_queue_view, TRUE, TRUE, 0);
    
    // Blocked Queue Section
    GtkWidget *blocked_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *blocked_label = create_styled_label("Blocked Queue", "queue-title-label");
    blocked_queue_view = create_styled_text_view(&blocked_queue_buffer, "blocked-queue");
    gtk_box_pack_start(GTK_BOX(blocked_box), blocked_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(blocked_box), blocked_queue_view, TRUE, TRUE, 0);
    
    // Add queues to horizontal box
    gtk_box_pack_start(GTK_BOX(queues_hbox), ready_box, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(queues_hbox), blocked_box, TRUE, TRUE, 0);
    
    // Running Process Section
    GtkWidget *running_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *running_label = create_styled_label("Running Process", "queue-title-label");
    running_process_view = create_styled_text_view(&running_process_buffer, "running-process");
    gtk_box_pack_start(GTK_BOX(running_box), running_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(running_box), running_process_view, TRUE, TRUE, 0);
    
    // Add all sections to main container
    gtk_box_pack_start(GTK_BOX(queues_box), queues_hbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(queues_box), running_box, TRUE, TRUE, 0);
    
    return queues_box;
}

// Create pending processes section
static GtkWidget* create_pending_section() {
    GtkWidget *pending_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_start(pending_box, 20);
    gtk_widget_set_margin_end(pending_box, 20);
    gtk_widget_set_margin_top(pending_box, 10);
    gtk_widget_set_margin_bottom(pending_box, 20);
    
    GtkWidget *pending_label = create_styled_label("Pending Processes", "queue-title-label");
    pending_processes_view = create_styled_text_view(&pending_processes_buffer, "pending-processes");
    
    gtk_box_pack_start(GTK_BOX(pending_box), pending_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pending_box), pending_processes_view, TRUE, TRUE, 0);
    
    return pending_box;
}

void create_gui(void) {
    // Initialize GTK
    gtk_init(NULL, NULL);

    // Create main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Main Dashboard");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 700);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create overlay for background
    GtkWidget *overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(window), overlay);

   

    // Main container for all UI elements
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(main_box, GTK_ALIGN_FILL);
    gtk_widget_set_valign(main_box, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(main_box, TRUE);
    gtk_widget_set_vexpand(main_box, TRUE);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), main_box);

    // Add header section
    gtk_box_pack_start(GTK_BOX(main_box), create_header_section(), FALSE, FALSE, 0);
    
    // Add separator
    GtkWidget *separator1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_start(separator1, 20);
    gtk_widget_set_margin_end(separator1, 20);
    gtk_box_pack_start(GTK_BOX(main_box), separator1, FALSE, FALSE, 10);
    
    // Add queues section
    gtk_box_pack_start(GTK_BOX(main_box), create_queues_section(), TRUE, TRUE, 0);
    
    // Add separator
    GtkWidget *separator2 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_start(separator2, 20);
    gtk_widget_set_margin_end(separator2, 20);
    gtk_box_pack_start(GTK_BOX(main_box), separator2, FALSE, FALSE, 10);
    
    // Add pending processes section
    gtk_box_pack_start(GTK_BOX(main_box), create_pending_section(), TRUE, TRUE, 0);

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