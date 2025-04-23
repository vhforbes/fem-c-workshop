#include <string.h>
#include <stdio.h>

const char* DEFAULT_FILE = "index.html";

char *to_path(char *req, size_t req_len) {
    char *start, *end;

    // Advance `start` to the first space
    for (start = req; start[0] != ' '; start++) {
        if (!start[0]) {
            return NULL;
        }
    }

    start++; // Skip over the space

    char *last_slash = NULL;
    char *last_dot = NULL;

    // Advance `end` to the second space
    for (end = start; end[0] != ' '; end++) {
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
    }

    // OPTIONS requests (and requests via proxies) may not start with '/'
    // For now, we don't support these requests. (We could always add support, though!)
    if (last_slash == NULL) {
        return NULL;
    }

    // If the path ends with a slash, default to index.html as the filename.
    if (last_dot == NULL || last_slash > last_dot) {
        last_slash++;

        // If there isn't enough room to copy in "index.html" then return NULL.
        // (This only happens if the request has no headers, which should only
        // come up in practice if the request is malformed or something.)
        if (last_slash + strlen(DEFAULT_FILE) > req + req_len) {
            return NULL;
        }

        // There will always be enough space here when receiving a request
        // from a normal browser, because after the target there will be " HTTP/1.1"
        // followed by a newline and at least one header. To be robust to requests
        // from other clients, we could accept the length of the request and only
        // do this if we have confirmed there's enough room.
        memcpy(last_slash, DEFAULT_FILE, strlen(DEFAULT_FILE) + 1);
    } else {
        end[0] = '\0';
    }

    return start + 1; // Skip the leading '/' (e.g. in "/blog/index.html")
}

int main() {
    char req1[] = "GET /blog HTTP/1.1\nHost: example.com";
    printf("Should be \"blog/index.html\": \"%s\"\n", to_path(req1, strlen(req1)));

    char req2[] = "GET /blog/ HTTP/1.1\nHost: example.com";
    printf("Should be \"blog/index.html\": \"%s\"\n", to_path(req2, strlen(req2)));

    char req3[] = "GET / HTTP/1.1\nHost: example.com";
    printf("Should be \"index.html\": \"%s\"\n", to_path(req3, strlen(req3)));

    char req4[] = "GET /blog ";
    printf("Should be \"(null)\": \"%s\"\n", to_path(req4, strlen(req4)));

    char req5[] = "GET /image.png HTTP/1.1\nHost: example.com";
    printf("Should be \"image.png\": \"%s\"\n", to_path(req5, strlen(req5)));

    char req6[] = "GET /static/image.png HTTP/1.1\nHost: example.com";
    printf("Should be \"static/image.png\": \"%s\"\n", to_path(req6, strlen(req6)));

    char req7[] = "GET /static/images/image.png HTTP/1.1\nHost: example.com";
    printf("Should be \"static/images/image.png\": \"%s\"\n", to_path(req7, strlen(req7)));

    return 0;
}
