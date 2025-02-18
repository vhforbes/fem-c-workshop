#include <string.h>
#include <unistd.h>

// To build and attempt to run the program: (it will give an error at first!)
//
// cc -o app3 3.c && ./app3

int main() {
    // 👉 First, fix this "`handle_req` was called before it was declared" error.
    //
    // There are two ways you can do it:
    // * Forward-declare `handle_req` by copying its first line up to the `{`,
    //   pasting it above `int main`, and adding `;` on the end.
    // * Moving the entire `handle_req` function above `main`'s declaration,
    //   so that `handle_req` is only called after it has already been declared.

    // 👉 Try calling this with a string that will fail parsing, like a string with
    // no spaces in it, or an empty string. What happens if there's no error handling?
    return handle_req("GET /blog HTTP/1.1 ...");
}

int handle_req(char *request) {
    // 👉 Handle errors by returning -1 if either `strchr` call returns NULL.
    char *start = strchr(request, ' ') + 1;
    char *end = strchr(start, ' ');

    write(1, "Target: ", 8);
    write(1, start, end - start);

    // 👉 Try changing `main` to print something if `handle_req` returned an error.
    return 0;
}
