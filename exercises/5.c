#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <errno.h>

#define PORT 8080
#define MAX_REQUEST_BYTES 32768

const char *OK_RESPONSE = "HTTP/1.1 200 OK\r\n\r\n";

int respond_error(int socket_fd, int fd, const char *message) {
    if (fd != -1) {
        close(fd);
    }

    char response[256]; // These error responses are small, e.g. "500 Internal Server Error"
    snprintf(response, sizeof(response), "HTTP/1.1 %s\r\n\r\n", message);

    return write(socket_fd, response, strlen(response)) != -1;
}

int respond_500(int socket_fd, int fd) {
    return respond_error(socket_fd, fd, "500 Internal Server Error");
}

int open_file_from_request(char *request, int socket_fd) {
    char *start = strchr(request, ' ') + 1;
    char *end = strchr(start, ' ');

    // OPTIONS requests (and requests via proxies) may not start with '/'
    // For now, we don't support these requests. (We could always add support, though!)
    if (end == NULL || start == NULL || *start != '/') {
        return respond_error(socket_fd, -1, "400 Bad Request");
    }

    char *path = start + 1; // Skip over the `/` so the file path becomes relative.

    // Null-terminate the path by writing a zero into the request string.
    if (end[-1] == '/') {
        // If the path ends with a slash, default to index.html as the filename.
        // There will always be enough space here when receiving a request
        // from a normal browser, because after the target there will be " HTTP/1.1"
        // followed by a newline and at least one header. To be robust to requests
        // from other clients, we could accept the length of the request and only
        // do this if we have confirmed there's enough room.
        memcpy(end, "index.html\0", 11);
    } else {
        // The path didn't end with a slash, so use it as a relative local path.
        // Replace the ' ' we know is here (from strchr) with a zero.
        *end = '\0';
    }

    int fd = open(path, O_RDONLY);

    if (fd == -1) {
        if (errno == ENOENT) {
            return respond_error(socket_fd, fd, "404 Not Found");
        } else {
            return respond_500(socket_fd, fd);
        }
    }

    return fd;
}

int handle_req(char *request, int socket_fd) {
    int fd = open_file_from_request(request, socket_fd);

    struct stat stats;

    // Populate the `stats` struct with the file's metadata
    // If it fails (even though the file was open), respond with a 500 error.
    if (fstat(fd, &stats) == -1) {
        return respond_500(socket_fd, fd);
    }

    // Write the header to the socket ("HTTP/1.1 200 OK" followed by a blank line)
    {
        ssize_t bytes_to_write = strlen(OK_RESPONSE);

        while (bytes_to_write) {
            ssize_t bytes_written = write(socket_fd, OK_RESPONSE, strlen(OK_RESPONSE));

            if (bytes_written == -1) {
                // If sending the 200 didn't succeed, the odds of 500 succeeding aren't great!
                return respond_500(socket_fd, fd);
            }

            bytes_to_write -= bytes_written;
        }
    }

    {
        // Send the file's contents to the socket
        ssize_t bytes_to_send = stats.st_size;

        while (bytes_to_send > 0) {
            // 👉 `sendfile` works differently on different operating systems!
            #ifdef __linux__
                ssize_t bytes_sent = sendfile(socket_fd, fd, NULL, stats.st_size);
                bool send_failed = bytes_sent == -1;
            #elif defined(__APPLE__)
                off_t bytes_sent = stats.st_size;
                bool send_failed = sendfile(fd, socket_fd, 0, &bytes_sent, NULL, 0) == -1;
            #else
                #error "Unsupported operating system"
            #endif

            if (send_failed) {
                // We already sent a 200 OK response, so it's too late to send a 500.
                break;
            }

            bytes_to_send -= bytes_sent;
        }
    }

    close(fd);

    return 0;
}

int main() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd == 0) {
        perror("Failed to open socket.");
        return -1;
    }

    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return -1;
    }

    if (listen(socket_fd, 3) < 0) {
        perror("listen");
        return -1;
    }

    printf("Listening on port %d\n", PORT);

    char request[MAX_REQUEST_BYTES] = {0};
    int addrlen = sizeof(address);
    int new_socket;

    // Loop forever to keep processing new connections
    while (1) {
        // Block until we get a connection on the socket
        if ((new_socket = accept(socket_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept() failed.");
            continue; // Continue listening for other connections even if accept fails
        }

        // Read all the bytes from the socket into the buffer
        ssize_t bytes_read = read(new_socket, request, MAX_REQUEST_BYTES);

        if (bytes_read < MAX_REQUEST_BYTES) {
            // Parse the URL and method out of the HTTP request
            handle_req(request, new_socket);
        } else {
            // The request was larger than the maximum size we support!
            respond_error(socket_fd, -1, "413 Content Too Large");
        }

        close(new_socket);
    }
}
