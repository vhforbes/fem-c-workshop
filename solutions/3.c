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
    if (end + strlen(DEFAULT_FILE) > req + req_len) {
        return NULL;
    }

    // Copy in "index.html", overwriting whatever was there in the request string.
    memcpy(end, DEFAULT_FILE, strlen(DEFAULT_FILE) + 1);

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

    return 0;
}
