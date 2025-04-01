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
// gcc -o app5 5.c && ./app5

#define PORT 8080
#define MAX_REQUEST_BYTES 32768

const char *OK_RESPONSE = "HTTP/1.1 200 OK\r\n\r\n";

int respond_error(int socket_fd, int fd, const char *message) {
    if (fd != -1) {
        close(fd);
    }

    char response[256]; // These error responses are small, e.g. "500 Internal Server Error"
    snprintf(response, sizeof(response), "HTTP/1.1 %s\r\n\r\n", message);

    write(socket_fd, response, strlen(response));

    return -1;
}

int respond_500(int socket_fd, int fd) {
    return respond_error(socket_fd, fd, "500 Internal Server Error");
}

const char* INDEX_HTML = "index.html";

char *to_path(char *req) {
    char *start = req;

    while (start[0] != ' ') {
        start++;

        if (start[0] == 0) {
            return NULL;
        }
    }

    start++;

    char *end = start;
    char *last_slash = NULL;
    char *last_dot = NULL;

    while(end[0] != ' ') {
        switch (end[0]) {
            case '/':
                last_slash = end;
                break;
            case '.':
                last_dot = end;
                break;
            case '\0':
                return NULL;
        }

        end++;
    }

    // OPTIONS requests (and requests via proxies) may not start with '/'
    // For now, we don't support these requests. (We could always add support, though!)
    if (last_slash == NULL) {
        return NULL;
    }

    // If there's no file extension, default to index.html as the filename.
    if (last_dot == NULL || last_slash > last_dot) {
        end[0] = '/';

        if (end > last_slash + 1) {
            end++;
        }
        // There will always be enough space here when receiving a request
        // from a normal browser, because after the target there will be " HTTP/1.1"
        // followed by a newline and at least one header. To be robust to requests
        // from other clients, we could accept the length of the request and only
        // do this if we have confirmed there's enough room.
        memcpy(end, INDEX_HTML, strlen(INDEX_HTML) + 1);
    } else {
        end[0] = '\0';
    }

    // Skip the leading '/'
    return start + 1;
}

struct response_header {
    char *string;
    size_t length;
};

// Macro to define a 4-char constant integer
#define FOURCHAR(a, b, c, d) (((uint32_t)(a)) | (((uint32_t)(b)) << 8) | \
 (((uint32_t)(c)) << 16) | (((uint32_t)(d)) << 24))

// Define constants for HTML and JPEG tags
const int HTML = FOURCHAR('h', 't', 'm', 'l');
const int WASM = FOURCHAR('w', 'a', 's', 'm');
const int WEBP = FOURCHAR('w', 'e', 'b', 'p');
const int JPEG = FOURCHAR('j', 'p', 'e', 'g');
const int JPG = FOURCHAR('j', 'p', 'g', 0);
const int CSS = FOURCHAR('c', 's', 's', 0);
const int PNG = FOURCHAR('p', 'n', 'g', 0);
const int GIF = FOURCHAR('g', 'i', 'f', 0);
const int JS = FOURCHAR('j', 's', 0, 0);

size_t write_response_header(char ext[4], char *resp_str) {
    char *content_type;

    switch (*(int *)ext) {
        case HTML:
            content_type = "text/html";
            break;
        case WASM:
            content_type = "application/wasm";
            break;
        case CSS:
            content_type = "text/css";
            break;
        case JS:
            content_type = "application/javascript";
            break;
        case PNG:
            content_type = "image/png";
            break;
        case JPG:
            content_type = "image/jpeg";
            break;
        case JPEG:
            content_type = "image/jpeg";
            break;
        case WEBP:
            content_type = "image/webp";
            break;
        case GIF:
            content_type = "image/gif";
            break;
        default:
            content_type = "application/octet-stream";
            break;
    }

    return sprintf(resp_str, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", content_type);
}

int handle_req(char *request, int socket_fd) {
    char *path = to_path(request);

    if (path == NULL) {
        return respond_error(socket_fd, -1, "400 Bad Request");
    }

    int fd = open(path, O_RDONLY);

    if (fd == -1) {
        if (errno == ENOENT) {
            return respond_error(socket_fd, fd, "404 Not Found");
        } else {
            return respond_500(socket_fd, fd);
        }
    }

    struct stat stats;

    // Populate the `stats` struct with the file's metadata
    // If it fails (even though the file was open), respond with a 500 error.
    if (fstat(fd, &stats) == -1) {
        return respond_500(socket_fd, fd);
    }

    // Write the header to the socket ("HTTP/1.1 200 OK" followed by a Content-Type header)
    {
        char ext[4] = {0};

        {
            char *path_ext = strrchr(path, '.'); // Could skip the strrchr() if done in to_path
            size_t ext_len = strlen(path_ext);

            // If the extension is less than 4 bytes (5, counting the '.'), set it to NULL.
            if (path_ext != NULL && ext_len <= 5) {
                strncpy(ext, path_ext + 1, ext_len - 1);
            }
        }

        struct response_header resp;
        char resp_str[1024]; // Our responses are short, e.g. "200 OK\n\nContent-Type: text/css"
        size_t resp_length = write_response_header(ext, resp_str);
        ssize_t bytes_to_write = resp_length;
        ssize_t bytes_written = 0;

        while (bytes_to_write) {
            bytes_written = write(socket_fd, resp_str + bytes_written, resp_length);

            if (bytes_written == -1) {
                // If sending the 200 didn't succeed, the odds of 500 succeeding aren't great!
                return respond_500(socket_fd, fd);
            }

            bytes_to_write -= bytes_written;
        }
    }

    {
        // Send the file's contents to the socket as the response body
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

    if (socket_fd == -1) {
        perror("Failed to open socket.");
        return -1;
    }

    // Prevent "Address in use" errors when restarting the server
    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        return -1;
    }

    struct sockaddr_in address; // IPv4 address

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("bind failed");
        return -1;
    }

    if (listen(socket_fd, 4) == -1) {
        perror("listen");
        return -1;
    }

    printf("Listening on port %d\n", PORT);

    char req[MAX_REQUEST_BYTES] = {0};
    int addrlen = sizeof(address);

    // Loop forever to keep processing new connections
    while (1) {
        // Block until we get a connection on the socket
        int req_socket_fd = accept(socket_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

        if (req_socket_fd >= 0) {
            // Read all the bytes from the socket into the buffer
            ssize_t bytes_read = read(req_socket_fd, req, MAX_REQUEST_BYTES);

            if (bytes_read < MAX_REQUEST_BYTES) {
                // Parse the URL and method out of the HTTP request
                handle_req(req, req_socket_fd);
            } else {
                // The request was larger than the maximum size we support!
                respond_error(socket_fd, -1, "413 Content Too Large");
            }

            close(req_socket_fd);
        } else {
            perror("accept() failed.");
            // Continue listening for other connections even if accept fails
        }
    }
}
