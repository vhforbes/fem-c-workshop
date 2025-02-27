#include <stdio.h>
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

// 👉 First, build and run the program.
//
// To do this, make sure you're in the `exercises` directory, and then run:
//
// cc -o app5 5.c && ./app5

const int PORT = 8080;
const int MAX_REQUEST_BYTES = 32768;

const char* DEFAULT_FILE = "index.html";

char *to_path(char *req) {
    char *start, *end;

    // Advance `start` to the first space
    for (start = req; start[0] != ' '; start++) {
        if (!start[0]) {
            return NULL;
        }
    }

    start++; // Skip over the space

    // Advance `end` to the second space
    for (end = start; end[0] != ' '; end++) {
        if (!end[0]) {
            return NULL;
        }
    }

    // Ensure there's a '/' right before where we're about to copy in "index.html"
    if (end[-1] != '/') {
        end[0] = '/';
        end++;
    }

    // If there isn't enough room to copy in "index.html" then return NULL.
    // (This only happens if the request has no headers, which should only
    // come up in practice if the request is malformed or something.)
    if (end + strlen(DEFAULT_FILE) > req + strlen(req)) {
        return NULL;
    }

    // Copy in "index.html", overwriting whatever was there in the request string.
    memcpy(end, DEFAULT_FILE, strlen(DEFAULT_FILE) + 1);

    return start + 1; // Skip the leading '/' (e.g. in "/blog/index.html")
}

int handle_req(char *request, int socket_fd) {
    char *path = to_path(request);

    if (path == NULL) {
        // 👉 Change this to send an the actual response to the socket.
        printf("HTTP/1.1 400 Bad Request\n\n");
        return -1;
    }

    int fd = open(path, O_RDONLY);

    if (fd == -1) {
        // 👉 Change this to send an the actual response to the socket.
        if (errno == ENOENT) {
            // This one is easy to try out in a browser: visit something like
            // http://localhost:8080/foo (which doesn't exist, so it will 404.)
            //
            // Is the output in your terminal different from what you expected?
            // If so, you can get a clue to what's happening if you run this in a
            // different terminal window, while watching the output of your C program:
            //
            // wget http://localhost:8080/foo
            printf("HTTP/1.1 404 Not Found\n\n");
        } else {
            printf("HTTP/1.1 500 Internal Server Error\n\n");
        }

        return -1;
    }

    struct stat stats;

    // Populate the `stats` struct with the file's metadata
    // If it fails (even though the file was open), respond with a 500 error.
    if (fstat(fd, &stats) == -1) {
        // 👉 Change this to send an the actual response to the socket.
        printf("HTTP/1.1 500 Internal Server Error\n\n");
    }

    // Write the header to the socket ("HTTP/1.1 200 OK")
    {
        const char *OK = "HTTP/1.1 200 OK\n\n";
        size_t bytes_written = 0;
        size_t bytes_to_write = strlen(OK);

        while (bytes_to_write) {
            bytes_written = write(socket_fd, OK + bytes_written, bytes_to_write);

            if (bytes_written == -1) {
                // 👉 Change this to send an the actual response to the socket.
                printf("HTTP/1.1 500 Internal Server Error\n\n");
                return -1;
            }

            bytes_to_write -= bytes_written;
        }
    }

    {
        // Read from the file and write to the socket
        char buffer[4096]; // Buffer for reading file data
        ssize_t bytes_read;

        // Loop until we've read the entire file
        while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
            ssize_t bytes_written = 0;
            ssize_t bytes_remaining = bytes_read;

            // Ensure all bytes are written to the socket
            while (bytes_remaining > 0) {
                ssize_t result = write(socket_fd, buffer + bytes_written, bytes_remaining);

                if (result == -1) {
                    // 👉 Change this to send an the actual response to the socket.
                    printf("HTTP/1.1 500 Internal Server Error\n\n");
                    return -1;
                }

                bytes_written += result;
                bytes_remaining -= result;
            }
        }

        if (bytes_read == -1) {
            // 👉 Change this to send an the actual response to the socket.
            printf("HTTP/1.1 500 Internal Server Error\n\n");
            return -1;
        }
    }

    close(fd);

    return 0;
}

int main() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd == -1) {
        printf("opening socket failed.\n");
        return -1;
    }

    // Prevent "Address in use" errors when restarting the server
    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        printf("setting socket options failed.\n");
        return -1;
    }

    struct sockaddr_in address; // IPv4 address

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        printf("bind() failed.\n");
        return -1;
    }

    if (listen(socket_fd, 4) == -1) {
        printf("listen() failed.\n");
        return -1;
    }

    printf("Listening on port %d\n", PORT);

    char req[MAX_REQUEST_BYTES + 1]; // + 1 for null terminator
    int addrlen = sizeof(address);

    // Loop forever to keep processing new connections
    while (1) {
        // Block until we get a connection on the socket
        int req_socket_fd = accept(socket_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

        if (req_socket_fd >= 0) {
            // Read all the bytes from the socket into the buffer
            ssize_t bytes_read = read(req_socket_fd, req, MAX_REQUEST_BYTES);

            if (bytes_read < MAX_REQUEST_BYTES) {
                req[bytes_read] = '\0'; // Null-terminate

                // Parse the URL and method out of the HTTP request
                handle_req(req, req_socket_fd);
            } else {
                // The request was larger than the maximum size we support!

                // 👉 Change this to send an the actual response to the socket.
                printf("HTTP/1.1 413 Content Too Large\n\n");
            }

            close(req_socket_fd);
        } else {
            // Continue listening for other connections even if accept fails
        }
    }
}
