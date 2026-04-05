#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 80
#define BUF_SIZE 4096

int main() {
    int sockfd;
    struct hostent *server;
    struct sockaddr_in server_addr;
    char request[256];
    char buffer[BUF_SIZE];
    ssize_t bytes_received;
    const char *hostname = "example.com";
    const char *path = "/";

    // Resolve the hostname to an IP address
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "gethostbyname: host not found\n");
        exit(EXIT_FAILURE);
    }

    printf("Connecting to %s (%s)...\n", hostname,
           inet_ntoa(*(struct in_addr *)server->h_addr_list[0]));

    // Create a TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Fill in the server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connected. Sending HTTP GET request...\n");

    // Build and send a simple HTTP GET request
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",
             path, hostname);

    if (write(sockfd, request, strlen(request)) == -1) {
        perror("write");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Read and print the response
    printf("Response from server:\n\n");
    while ((bytes_received = read(sockfd, buffer, BUF_SIZE - 1)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    if (bytes_received == -1) {
        perror("read");
    }

    close(sockfd);
    printf("\nConnection closed.\n");

    return 0;
}
