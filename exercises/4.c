#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

// 👉 First, build and run the program.
//
// To do this, make sure you're in the `exercises` directory, and then run:
//
// gcc -o app4 4.c && ./app4

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

void print_file(const char *path) {
    int fd = open(path, O_RDONLY);

    if(fd == -1) {
        printf("fd error");
        return;
    }

    struct stat metadata;
    if(fstat(fd, &metadata) == -1) {
        printf("fstat error");
        return;
    };

    // 👉 Change this to `char *` and malloc(). (malloc comes from <stdlib.h>)
    //    Hint 1: Don't forget to handle the case where malloc returns NULL!
    //    Hint 2: Don't forget to `free(buf)` later, to prevent memory leaks.
    char* buf = malloc(metadata.st_size + 1);

    if(buf == NULL) {
        printf("Error on buf malloc");

        // need to close the file or we'll have a memory leak
        // dont need to free mem cause none was allocated
        close(fd);
        return;
    }

    ssize_t bytes_read = read(fd, buf, metadata.st_size);

    if(bytes_read == -1) {
        printf("read error");

        // need to close file and free mem otherwise would have memory leak
        close(fd);
        free(buf);
        return;
    }

    buf[bytes_read] = '\0';
    printf("\n%s contents:\n\n%s\n", path, buf);

    close(fd);
    free(buf);

    // 👉 Go back and add error handling for all the cases above where errors could happen.
    //    (You can just printf that an error happened.) Some relevant docs:
    //
    //    https://www.man7.org/linux/man-pages/man2/open.2.html
    //    https://www.man7.org/linux/man-pages/man2/stat.2.html
    //    https://www.man7.org/linux/man-pages/man2/read.2.html
    //    https://www.man7.org/linux/man-pages/man3/malloc.3.html
}

int main() {
    char req1[] = "GET / HTTP/1.1\nHost: example.com";
    print_file(to_path(req1));

    char req2[] = "GET /blog HTTP/1.1\nHost: example.com";
    print_file(to_path(req2));


    return 0;
}
