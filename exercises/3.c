#include <string.h>
#include <stdio.h>

// 👉 First, build and run the program.
//
// To do this, make sure you're in the `exercises` directory, and then run:
//
// gcc -o app3 3.c && ./app3

// GET /blog HTTP/1.1\nHost: example.com

const char* DEFAULT_FILE = "index.html";

char *to_path(char *req) {
    char *start, *end;

    // Advance `start` to the first space
    for (start = req; start[0] != ' '; start++) {
        //-> start[0] != " " OR start[0] != '\0'
        if (!start[0]) { 
            return NULL;
        }
    }


    start+=2; // Skip over the space
    // start++; // Skip over the / ??


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
    

    // Copy in "index.html", overwriting whatever was there in the request string.
    if((size_t)end + strlen(DEFAULT_FILE) + 1 > (size_t)req + strlen(req)) {
        return NULL;
    }

    memcpy(
        end,
        DEFAULT_FILE,
        // 👉 Try removing the +1 here. Re-run to see what happens, but first try to guess!
        strlen(DEFAULT_FILE) + 1
    );

    return start;
}

int main() {
    // 👉 These three don't currently trim off the leading '/' - modify to_path to fix them!
    char req1[] = "GET /blog HTTP/1.1\nHost: example.com";
    printf("Should be \"blog/index.html\": \"%s\"\n", to_path(req1));

    char req2[] = "GET /blog/ HTTP/1.1\nHost: example.com";
    printf("Should be \"blog/index.html\": \"%s\"\n", to_path(req2));

    char req3[] = "GET / HTTP/1.1\nHost: example.com";
    printf("Should be \"index.html\": \"%s\"\n", to_path(req3));

    // 👉 Before fixing this next one, try moving it up to the beginning of main().
    //    What happens?

    // 👉 Finally, fix it by handling the case where `req` is too short to
    //    have "index.html" memcpy'd into it.
    //    Hint: `strlen()` returns an integer whose type is not `int` but rather `size_t`
    char req4[] = "GET /blog ";
    printf("Should be \"(null)\": \"%s\"\n", to_path(req4));

    return 0;
}
