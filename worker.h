// worker.h

#ifndef WORKER_H
#define WORKER_H

// A função que será o ponto de entrada para cada nova thread de cliente
void *handle_client_thread(void *arg);

#endif