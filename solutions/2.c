#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main() {
    char *header = "HTT\0P/1.1 200 OK";

    write(1, header, strlen(header));

    // 👉 Try changing this `%s` to `%zud` (ignore the compiler warning).
    printf("\n\nThat output was from write(). This is from printf: %s\n", (char*)0);

    return 0;
}
