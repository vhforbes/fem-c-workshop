#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024

int handle_http_request(char *request, int socket_fd) {
    // Parse the URL out of the HTTP request
    char *method, *url;
    method = strtok(request, " ");
    if (!method) return -1;

    url = strtok(NULL, " ");
    if (!url) return -1;

    printf("Received request for %s\n", url);

    // Open and read the corresponding file
    char file_path[1024];

    if (url[strlen(url) - 1] == '/') {
        snprintf(file_path, sizeof(file_path), ".%s/index.html", url);
    } else {
        snprintf(file_path, sizeof(file_path), ".%s", url);
    }

    int file_fd = open(file_path, O_RDONLY);
    if (file_fd < 0) {
        char error_message[1024];
        snprintf(error_message, sizeof(error_message), "File not found on disk: %s", file_path);
        perror(error_message);
        close(socket_fd);
        return -1;
    }

    char file_buffer[BUFFER_SIZE];
    ssize_t file_read = read(file_fd, file_buffer, BUFFER_SIZE);
    close(file_fd);

    // Send response
    if (file_read > 0) {
        char header[] = "HTTP/1.1 200 OK\r\n\r\n";
        write(socket_fd, header, strlen(header));
        write(socket_fd, file_buffer, file_read);
    } else {
        char header[] = "HTTP/1.1 404 Not Found\r\n\r\n";
        write(socket_fd, header, strlen(header));
    }

    return 0;
}
