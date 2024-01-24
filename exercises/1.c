// #include <stdio.h>
#include <unistd.h>

// 👉 First, build and run the program.
//
// To do this, make sure you're in the `exercises` directory, and then run:
//
// cc -o app1 1.c && ./app1

int main() {

    // 👉 Try changing the string here, and also the length (13 for the string "Hello, World!").
    //    What happens if you make the length too short? What happens if you make it too long? (😱)
    write(1, "Hello, World!", 13);

    // 👉 Try uncommenting the next 2 lines, as well as the #include <stdio.h> at the top.
    // 👉 Try adding a second number, named num2. Set it to something other than 42.
    // int num = 42;
    // printf("The number is: %d", num);

    // 👉 Try having printf print *both* numbers.
    //    Hint: you'll need to make 2 changes to printf's arguments to do this!

    // 👉 Try returning something other than 0. (To see it, you'll need to run `echo $?`
    //    immediately after the program finishes running.)
    return 0;
}
