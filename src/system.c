#include "defs.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// Helper functions just used by this C file to clean up our code
// Using static means they can't get linked into other files

static int system_convert(System *system);
static void system_simulate_process_time(System *system);
static int system_store_resources(System *system);

/**
 * Creates a new `System` object.
 *
 * Allocates memory for a new `System` and initializes its fields.
 * The `name` is dynamically allocated.
 *
 * @param[out] system          Pointer to the `System*` to be allocated and initialized.
 * @param[in]  name            Name of the system (the string is copied).
 * @param[in]  consumed        `ResourceAmount` representing the resource consumed.
 * @param[in]  produced        `ResourceAmount` representing the resource produced.
 * @param[in]  processing_time Processing time in milliseconds.
 * @param[in]  event_queue     Pointer to the `EventQueue` for event handling.
 */
void system_create(System **system, const char *name, ResourceAmount consumed, ResourceAmount produced, int processing_time, EventQueue *event_queue) {
    *system = (System *)malloc(sizeof(System));
    (*system)->name = strdup(name);
    (*system)->consumed = consumed;
    (*system)->produced = produced;
    (*system)->amount_stored = 0;
    (*system)->processing_time = processing_time;
    (*system)->status = STANDARD;
    (*system)->event_queue = event_queue;
}

/**
 * Destroys a `System` object.
 *
 * Frees all memory associated with the `System`.
 *
 * @param[in,out] system  Pointer to the `System` to be destroyed.
 */
void system_destroy(System *system) {
    if (system) {
        free(system->name);
        free(system);
    }
}


/**
 * Runs the main loop for a `System`.
 *
 * This function manages the lifecycle of a system, including resource conversion,
 * processing time simulation, and resource storage. It generates events based on
 * the success or failure of these operations.
 *
 * @param[in,out] system  Pointer to the `System` to run.
 */
void system_run(System *system) {
    Event event;
    int result_status;

    if (system->amount_stored == 0) {
        result_status = system_convert(system);
        if (result_status != STATUS_OK) {
            event_init(&event, system, system->consumed.resource, result_status, PRIORITY_HIGH, system->consumed.resource->amount);
            event_queue_push(system->event_queue, &event);
            usleep(SYSTEM_WAIT_TIME * 1000);
        }
    }

    if (system->amount_stored > 0) {
        result_status = system_store_resources(system);
        if (result_status != STATUS_OK) {
            event_init(&event, system, system->produced.resource, result_status, PRIORITY_LOW, system->produced.resource->amount);
            event_queue_push(system->event_queue, &event);
            usleep(SYSTEM_WAIT_TIME * 1000);
        }
    }
}

/**
 * Converts resources in a `System`.
 *
 * Handles the consumption of required resources and simulates processing time.
 * Updates the amount of produced resources based on the system's configuration.
 *
 * @param[in,out] system           Pointer to the `System` performing the conversion.
 * @param[out]    amount_produced  Pointer to the integer tracking the amount of produced resources.
 * @return                         `STATUS_OK` if successful, or an error status code.
 */
static int system_convert(System *system) {
    Resource *consumed_resource = system->consumed.resource;
    int amount_consumed = system->consumed.amount;

    if (!consumed_resource) {
        return STATUS_OK;
    }

    sem_wait(&consumed_resource->lock);  
    if (consumed_resource->amount >= amount_consumed) {
        consumed_resource->amount -= amount_consumed;
        sem_post(&consumed_resource->lock);  
        system_simulate_process_time(system);
        if (system->produced.resource) {
            system->amount_stored += system->produced.amount;
        }
        return STATUS_OK;
    } else {
        sem_post(&consumed_resource->lock);  
        return (consumed_resource->amount == 0) ? STATUS_EMPTY : STATUS_INSUFFICIENT;
    }
}

/**
 * Simulates the processing time for a `System`.
 *
 * Adjusts the processing time based on the system's current status (e.g., SLOW, FAST)
 * and sleeps for the adjusted time to simulate processing.
 *
 * @param[in] system  Pointer to the `System` whose processing time is being simulated.
 */
static void system_simulate_process_time(System *system) {
    int adjusted_processing_time = system->processing_time;

    switch (system->status) {
        case SLOW:
            adjusted_processing_time *= 2;
            break;
        case FAST:
            adjusted_processing_time /= 2;
            break;
        default:
            break;
    }

    usleep(adjusted_processing_time * 1000);
}

/**
 * Stores produced resources in a `System`.
 *
 * Attempts to add the produced resources to the corresponding resource's amount,
 * considering the maximum capacity. Updates the `produced_resource_count` to reflect
 * any leftover resources that couldn't be stored.
 *
 * @param[in,out] system                   Pointer to the `System` storing resources.
 * @param[in,out] produced_resource_count  Pointer to the integer value of how many resources need to be stored, updated with the amount that could not be stored.
 * @return                                 `STATUS_OK` if all resources were stored, or `STATUS_CAPACITY` if not all could be stored.
 */
static int system_store_resources(System *system) {
    Resource *produced_resource = system->produced.resource;

    if (!produced_resource || system->amount_stored == 0) {
        return STATUS_OK;
    }

    sem_wait(&produced_resource->lock); 

    int available_space = produced_resource->max_capacity - produced_resource->amount;

    if (available_space >= system->amount_stored) {
        produced_resource->amount += system->amount_stored;
        system->amount_stored = 0;
        sem_post(&produced_resource->lock); 
        return STATUS_OK;
    } else if (available_space > 0) {
        produced_resource->amount += available_space;
        system->amount_stored -= available_space;
    }

    sem_post(&produced_resource->lock);  
    return STATUS_CAPACITY;
}


/**
 * Initializes the `SystemArray`.
 *
 * Allocates memory for the array of `System*` pointers of capacity 1 and sets up initial values.
 *
 * @param[out] array  Pointer to the `SystemArray` to initialize.
 */
void system_array_init(SystemArray *array) {
    array->systems = (System **)malloc(sizeof(System *) * 1);
    array->size = 0;
    array->capacity = 1;
}

/**
 * Cleans up the `SystemArray` by destroying all systems and freeing memory.
 *
 * Iterates through the array, cleaning any memory for each System pointed to by the array.
 *
 * @param[in,out] array  Pointer to the `SystemArray` to clean.
 */
void system_array_clean(SystemArray *array) {
    for (int i = 0; i < array->size; i++) {
        system_destroy(array->systems[i]);
    }
    free(array->systems);
}

/**
 * Adds a `System` to the `SystemArray`, resizing if necessary (doubling the size).
 *
 * Resizes the array when the capacity is reached and adds the new `System`.
 * Use of realloc is NOT permitted.
 *
 * @param[in,out] array   Pointer to the `SystemArray`.
 * @param[in]     system  Pointer to the `System` to add.
 */
void system_array_add(SystemArray *array, System *system) {
    if (array->size == array->capacity) {
        array->capacity *= 2;
        System **new_array = (System **)malloc(sizeof(System *) * array->capacity);
        for (int i = 0; i < array->size; i++) {
            new_array[i] = array->systems[i];
        }
        free(array->systems);
        array->systems = new_array;
    }
    array->systems[array->size++] = system;
}

void *system_thread(void *arg) {
    System *system = (System *)arg;
    while (system->status != TERMINATE) {
        system_run(system);
    }
    return NULL;
}