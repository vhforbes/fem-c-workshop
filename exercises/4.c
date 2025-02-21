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

int respond_error(int fd, const char *message) {
    if (fd != -1) {
        close(fd);
    }

    char response[256]; // These error responses are small, e.g. "500 Internal Server Error"
    snprintf(response, sizeof(response), "HTTP/1.1 %s\r\n\r\n", message);

    return write(stderr, response, strlen(response)) != -1;
}

int open_file_from_request(char *request) {
    char *start = strchr(request, ' ') + 1;
    char *end = strchr(start, ' ');

    // OPTIONS requests (and requests via proxies) may not start with '/'
    // For now, we don't support these requests. (We could always add support, though!)
    if (end == NULL || start == NULL || *start != '/') {
        return respond_error(-1, "400 Bad Request");
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
            return respond_error(fd, "404 Not Found");
        } else {
            return respond_error(fd, "500 Internal Server Error");
        }
    }

    return fd;
}

int handle_req(char *request) {
    int fd = open_file_from_request(request);

    struct stat stats;

    // Populate the `stats` struct with the file's metadata
    // If it fails (even though the file was open), respond with a 500 error.
    if (fstat(fd, &stats) == -1) {
        fprintf(stderr, "500 Internal Server Error\n");
        return -1;
    }

    // TODO print the file size I guess

    close(fd);

    return 0;
}

int main() {
    // Do file stuff!
}
