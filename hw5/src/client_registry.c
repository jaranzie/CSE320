//
// Created by jaranzie on 12/4/22.
//
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>


#include "../include/client_registry.h"

/*
 * Initialize a new client registry.
 *
 * @return  the newly initialized client registry, or NULL if initialization
 * fails.
 */

typedef struct node {
    int fd;
    struct node* next;
} node;

struct client_registry {
    pthread_mutex_t mutex;
    sem_t shutdown;
    node* head;
    int client_count;
};

CLIENT_REGISTRY *creg_init() {
    CLIENT_REGISTRY* registry = malloc(sizeof(CLIENT_REGISTRY));

    /*init mutex*/
    pthread_mutex_init(&registry->mutex, NULL);

    /* init semaphore*/
    sem_init(&registry->shutdown,0,0);

    registry->client_count = 0;

    /*init linked list*/
    registry->head = NULL;

    return registry;
}

/*
 * Finalize a client registry, freeing all associated resources.
 *
 * @param cr  The client registry to be finalized, which must not
 * be referenced again.
 */
void creg_fini(CLIENT_REGISTRY *cr) {
    sem_destroy(&cr->shutdown);
    pthread_mutex_destroy(&cr->mutex);
    free(cr);
    return;
}

/*
 * Register a client file descriptor.
 *
 * @param cr  The client registry.
 * @param fd  The file descriptor to be registered.
 * @return 0 if registration succeeds, otherwise -1.
 */
int creg_register(CLIENT_REGISTRY *cr, int fd) {
    if(pthread_mutex_lock(&cr->mutex)) {
        return -1; /*indicates error with mutex*/
    }
    cr->client_count += 1;
    node* new_client = malloc(sizeof(node));
    new_client->next = NULL;
    new_client->fd = fd;
    if(cr->head == NULL) {
        cr->head = new_client;
    } else {
        node* head = cr->head;
        while(head->next != NULL) {
            head = head->next;
        }
        head->next = new_client;
    }

    if(pthread_mutex_unlock(&cr->mutex)) {
        return -1; /*indicates error with mutex*/
    }
    return 0;
}

/*
 * Unregister a client file descriptor, alerting anybody waiting
 * for the registered set to become empty.
 *
 * @param cr  The client registry.
 * @param fd  The file descriptor to be unregistered.
 * @return 0  if unregistration succeeds, otherwise -1.
 */
int creg_unregister(CLIENT_REGISTRY *cr, int fd) {
    if(pthread_mutex_lock(&cr->mutex)) {
        return -1; /*indicates error with mutex*/
    }
    if(cr->client_count <= 0) {
        return -1;
    }
    cr->client_count--;
    node* head = cr->head;
    if(head->fd == fd) {
        cr->head = head->next;
    } else {
        node* prev = head;
        head = head->next;
        while(head->next != NULL) {
            if(head->fd == fd) {
                prev->next = head->next;
                break;
            } else if (head->next == NULL) {
                return -1;
            }
            head = head->next;
            prev = prev->next;
        }
    }
    shutdown(head->fd, SHUT_RD);
//maybe close fd?
    free(head);
    if(pthread_mutex_unlock(&cr->mutex)) {
        return -1; /*indicates error with mutex*/
    }
    return 0;
}

/*
 * A thread calling this function will block in the call until
 * the number of registered clients has reached zero, at which
 * point the function will return.
 *
 * @param cr  The client registry.
 */
void creg_wait_for_empty(CLIENT_REGISTRY *cr) {
    sem_wait(&cr->shutdown);
    return;
}

/*
 * Shut down all the currently registered client file descriptors.
 *
 * @param cr  The client registry.
 */
void creg_shutdown_all(CLIENT_REGISTRY *cr) {
    pthread_mutex_lock(&cr->mutex);
    while(cr->head != NULL) {
        creg_unregister(cr, cr->head->fd);
    }
    sem_post(&cr->shutdown);
    pthread_mutex_unlock(&cr->mutex);
}
