#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFF_SIZE 1024

void *client_handler(void *client_socket_ptr) {
    int client_socket = *(int *)client_socket_ptr;
    free(client_socket_ptr);
    char buffer[BUFF_SIZE];
    ssize_t received_bytes;

    while ((received_bytes = recv(client_socket, buffer, BUFF_SIZE, 0)) > 0) {
        send(client_socket, buffer, received_bytes, 0); 
        // Echo back to client
    }

    close(client_socket); 
    // Close client connection
    return NULL;
}

// Function to initialize the server and start listening
int start_server(int port) {
    int server_socket;
    struct sockaddr_in server_address;

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Prepare the sockaddr_in structure
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Check for connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);
    return server_socket;
}

// Main server loop
void accept_connections(int server_socket) {
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    while (1) {
        // Accept incoming client connections
        int *client_socket = malloc(sizeof(int));
        *client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (*client_socket < 0) {
            perror("Accept failed");
            free(client_socket);
            continue;
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Create new thread for each client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, client_handler, client_socket) != 0) {
            perror("Thread creation failed");
            free(client_socket);
        }

        pthread_detach(thread_id); // Detach thread for automatic cleanup
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3 || strcmp(argv[1], "-p") != 0) {
        fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);
    int server_socket = start_server(port); // Initialize and start the server
    accept_connections(server_socket); // Begin accepting client connections

    close(server_socket); // Close the server socket when done
    return 0;
}
