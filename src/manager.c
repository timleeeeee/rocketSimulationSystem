#include "defs.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// This function is only used by this file, so declared here and set to static to avoid having it linked by any other file

static void display_simulation_state(Manager *manager);

/**
 * Initializes the `Manager`.
 *
 * Sets up the manager by initializing the system array, resource array, and event queue.
 * Prepares the simulation to be run.
 *
 * @param[out] manager  Pointer to the `Manager` to initialize.
 */
void manager_init(Manager *manager) {
    manager->simulation_running = 1; 
    system_array_init(&manager->system_array);
    resource_array_init(&manager->resource_array);
    event_queue_init(&manager->event_queue);
}

/**
 * Cleans up the `Manager`.
 *
 * Frees all resources associated with the manager.
 *
 * @param[in,out] manager  Pointer to the `Manager` to clean.
 */
void manager_clean(Manager *manager) {
    system_array_clean(&manager->system_array);
    resource_array_clean(&manager->resource_array);
    event_queue_clean(&manager->event_queue);
}


/**
 * Runs the manager loop.
 *
 * Handles event processing, updates system statuses, and displays the simulation state.
 * Continues until the simulation is no longer running. (In a multi-threaded implementation)
 *
 * @param[in,out] manager  Pointer to the `Manager`.
 */
void manager_run(Manager *manager) {
    Event event;
    int event_found_flag = 0;
    int no_oxygen_flag = 0, distance_reached_flag = 0, need_more_flag = 0, need_less_flag = 0;
    int status = STANDARD;

    display_simulation_state(manager);

    event_found_flag = event_queue_pop(&manager->event_queue, &event);

    while (event_found_flag) {
        printf("Event: [%s] Resource [%s : %d] Status [%d]\n",
               event.system->name, event.resource->name, event.amount, event.status);

        no_oxygen_flag = (event.status == STATUS_EMPTY && strcmp(event.resource->name, "Oxygen") == 0);
        distance_reached_flag = (event.status == STATUS_CAPACITY && strcmp(event.resource->name, "Distance") == 0);
        need_more_flag = (event.status == STATUS_LOW || event.status == STATUS_EMPTY || event.status == STATUS_INSUFFICIENT);
        need_less_flag = (event.status == STATUS_CAPACITY);

        if (no_oxygen_flag) {
            printf("Oxygen depleted. Terminating all systems.\n");
            status = TERMINATE;
            manager->simulation_running = 0;
        } else if (distance_reached_flag) {
            printf("Destination reached. Terminating all systems.\n");
            status = TERMINATE;
            manager->simulation_running = 0;
        } else if (need_more_flag) {
            status = FAST;
        } else if (need_less_flag) {
            status = SLOW;
        }

        if (no_oxygen_flag || distance_reached_flag || need_more_flag || need_less_flag) {
            for (int i = 0; i < manager->system_array.size; i++) {
                System *current_system = manager->system_array.systems[i];
                if (status == TERMINATE || current_system->produced.resource == event.resource) {
                    current_system->status = status;
                }
            }
        }

        event_found_flag = event_queue_pop(&manager->event_queue, &event);
    }
}

// Don't worry much about these! These are special codes that allow us to do some formatting in the terminal
// Such as clearing the line before printing or moving the location of the "cursor" that will print.
#define ANSI_CLEAR "\033[2J"
#define ANSI_MV_TL "\033[H"
#define ANSI_LN_CLR "\033[K"
#define ANSI_MV_D1 "\033[1B"
#define ANSI_SAVE "\033[s"
#define ANSI_RESTORE "\033[u"

/**
 * Displays the current simulation state.
 *
 * Outputs the statuses of resources and systems to the console.
 * This function is typically called periodically to update the display.
 *
 * @param[in] manager  Pointer to the `Manager` containing the simulation state.
 */
static void display_simulation_state(Manager *manager) {
    printf(ANSI_CLEAR ANSI_MV_TL);

    printf(ANSI_LN_CLR "Current Resource Amounts:\n");
    printf(ANSI_LN_CLR "-------------------------\n");

    for (int i = 0; i < manager->resource_array.size; i++) {
        Resource *resource = manager->resource_array.resources[i];
        printf(ANSI_LN_CLR "%s: %d / %d\n", resource->name, resource->amount, resource->max_capacity);
    }

    printf(ANSI_LN_CLR "\nSystem Statuses:\n");
    printf(ANSI_LN_CLR "----------------\n");

    for (int i = 0; i < manager->system_array.size; i++) {
        System *system = manager->system_array.systems[i];
        const char *status_str = "UNKNOWN";

        switch (system->status) {
            case TERMINATE: status_str = "TERMINATE"; break;
            case DISABLED: status_str = "DISABLED"; break;
            case SLOW: status_str = "SLOW"; break;
            case STANDARD: status_str = "STANDARD"; break;
            case FAST: status_str = "FAST"; break;
        }

        printf(ANSI_LN_CLR "%s: %s\n", system->name, status_str);
    }

    fflush(stdout);
}

void *manager_thread(void *arg) {
    Manager *manager = (Manager *)arg;
    while (manager->simulation_running) {
        manager_run(manager);
        usleep(MANAGER_WAIT_TIME * 1000);  
    }
    return NULL;
}