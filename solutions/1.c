#include <stdio.h>
#include <unistd.h>

int main() {
    write(1, "Hello, World!\n", 14);

    int num = 42;
    int num2 = 1337;
    printf("The number is: %d\nThe second number is: %d\n", num, num2);

    return 101;
}
