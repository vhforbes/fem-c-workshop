#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 32768

void parse_http_request(char *buffer, char **method, char **url) {
    *method = strtok(buffer, " ");
    if (*method) {
        *url = strtok(NULL, " ");
    }
}

void url_to_path(const char *url, char *file_path, size_t file_path_size) {
    if (url[strlen(url) - 1] == '/') {
        snprintf(file_path, file_path_size, ".%s/index.html", url);
    } else {
        snprintf(file_path, file_path_size, ".%s", url);
    }
}

int url_to_response(const char *url, char *response, size_t response_size) {
    char file_path[1024];
    url_to_path(url, file_path, sizeof(file_path));

    int file_fd = open(file_path, O_RDONLY);

    const char *status;
    const char *content_type;
    char body[BUFFER_SIZE];
    ssize_t body_len;

    if (file_fd < 0) {
        status = "404 Not Found";
        content_type = "text/plain";
        body_len = snprintf(body, sizeof(body), "404 Not Found: The requested file was not found on this server.");
    } else {
        status = "200 OK";
        content_type = "text/html";
        body_len = read(file_fd, body, sizeof(body));
        close(file_fd);

        if (body_len < 0) {
            status = "500 Internal Server Error";
            content_type = "text/plain";
            body_len = snprintf(body, sizeof(body), "500 Internal Server Error: An error occurred while reading the file.");
        }
    }

    char header[BUFFER_SIZE];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %s\r\nContent-Type: %s\r\n\r\n", status, content_type);

    if (header_len + body_len > response_size) {
        fprintf(stderr, "Response buffer too small\n");
        return -1;
    }

    memcpy(response, header, header_len);
    memcpy(response + header_len, body, body_len);

    return header_len + body_len;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Set up the socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind to port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        return -1;
    }

    printf("Listening on port %d\n", PORT);

    // Loop forever to keep processing new connections
    while (1) {
        // Block until we get a connection on the socket
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue; // Continue listening for other connections even if accept fails
        }

        // Read all the bytes from the socket into the buffer
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t valread = read(new_socket, buffer, BUFFER_SIZE);

        // Parse the URL and method out of the HTTP request
        char *method, *url;
        parse_http_request(buffer, &method, &url);

        if (method && url) {
            printf("Received request for %s\n", url);

            char response[BUFFER_SIZE * 2];  // Increased size to accommodate both header and file content
            int response_length = url_to_response(url, response, sizeof(response));

            if (response_length > 0) {
                write(new_socket, response, response_length);
            }
        }

        if (method) {
            free(method);
        }

        if (url) {
            free(url);
        }

        close(new_socket);
    }
}


// TODO next:
// build up exercises to the first 3 functions and then finally the last function
// see if we can avoid snprintf, fprintf, printf, etc. in favor of write() and concat etc.
