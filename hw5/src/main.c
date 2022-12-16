#include <stdlib.h>

#include "client_registry.h"
#include "exchange.h"
#include "account.h"
#include "server.h"
#include "trader.h"
#include "debug.h"

#include <signal.h>
#include "csapp.h"

extern EXCHANGE *exchange;
extern CLIENT_REGISTRY *client_registry;

static void terminate(int status);

//static int sighup_flag = 0;
void sighup_handler(int signal) {
    terminate(0);
}
/*
 * "Bourse" exchange server.
 *
 * Usage: bourse <port>
 */
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    char* port;
    int opt = getopt(argc, argv, "p:");
    if (opt == 'p') {
        port = optarg;
    } else {
        fprintf(stderr, "Usage: %s -p <Port>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }
    struct sigaction sa;
    sa.sa_handler = sighup_handler;
    sigaction(SIGHUP, &sa, NULL);
    // Perform required initializations of the client_registry,
    // maze, and player modules.
    client_registry = creg_init();
    accounts_init();
    traders_init();
    exchange = exchange_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function brs_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    int listenfd = Open_listenfd(port);
    int connfd;
    while (1) {
        connfd = accept(listenfd, NULL, NULL);
        void* connfd_p = malloc(sizeof(int));
        *((int*)connfd_p) = connfd;
        pthread_t conn_thread;
        pthread_create(&conn_thread, NULL, brs_client_service, connfd_p);
    }

    fprintf(stderr, "You have to finish implementing main() "
	    "before the Bourse server will function.\n");

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);
    
    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    exchange_fini(exchange);
    traders_fini();
    accounts_fini();

    debug("Bourse server terminating");
    exit(status);
}
