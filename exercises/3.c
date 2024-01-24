#include <string.h>
#include <stdio.h>

// 👉 First, build and run the program.
//
// To do this, make sure you're in the `exercises` directory, and then run:
//
// cc -o app3 3.c && ./app3

int handle_http_request(char *request);

int main() {
    char* request = "GET https://frontendmasters.com";

    return handle_http_request(request);
}

int handle_http_request(char *request) {
    // Parse the URL out of the HTTP request
    char *method, *url;
    method = strtok(request, " ");
    if (!method) return -1;

    url = strtok(NULL, " ");
    if (!url) return -1;

    printf("Received request for %s\n", url);

    return 0;
}
