#ifndef MUTEX_TAB_H
#define MUTEX_TAB_H

#include <gtk/gtk.h>
#include "main.h" // Include the header file where Mutex and Queue are defined

// Declare the global mutex grid
extern GtkGrid *mutex_grid;

// Function declarations
GtkWidget* create_mutex_tab(GtkGrid **out_grid);
void refresh_mutex_tab(GtkGrid *mutex_grid);

#endif // MUTEX_TAB_H