#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT "80"
#define BUF_SIZE 4096

int main() {
    int sockfd;
    struct addrinfo hints, *res, *rp;
    char ip_str[INET6_ADDRSTRLEN];
    char request[256];
    char buffer[BUF_SIZE];
    ssize_t bytes_received;
    const char *hostname = "example.com";
    const char *path = "/";
    int status;

    // Resolve the hostname to an IP address (supports IPv4 and IPv6)
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, PORT, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    // Try each address until a successful connect
    sockfd = -1;
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        void *addr;
        if (rp->ai_family == AF_INET) {
            addr = &((struct sockaddr_in *)rp->ai_addr)->sin_addr;
        } else {
            addr = &((struct sockaddr_in6 *)rp->ai_addr)->sin6_addr;
        }
        inet_ntop(rp->ai_family, addr, ip_str, sizeof(ip_str));

        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1) {
            fprintf(stderr, "socket creation failed for %s\n", hostname);
            continue;
        }

        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            printf("Connecting to %s (%s)...\n", hostname, ip_str);
            break;
        }

        close(sockfd);
        sockfd = -1;
    }

    freeaddrinfo(res);

    if (sockfd == -1) {
        fprintf(stderr, "connect: could not connect to %s\n", hostname);
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
