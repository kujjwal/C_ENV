//
// Created by Ujjwal Krishnamurthi on 8/21/20.
//

#include <unistd.h>
#include <stdio.h>
/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char **args);
int lsh_help();
int lsh_exit();

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str_args[] = {
        "cd"
};

char *builtin_str_noargs[] = {
        "help",
        "exit"
};

int (*builtin_func_args[]) (char **) = {
        &lsh_cd
};

int (*builtin_func_noargs[]) () = {
        &lsh_help,
        &lsh_exit
};

int lsh_num_builtins_args() {
    return sizeof(builtin_str_args) / sizeof(char *);
}

int lsh_num_builtins_noargs() {
    return sizeof(builtin_str_noargs) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int lsh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help() {
    printf("Ujjwal Krishnamurthi's LSH Shell\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (int i = 0; i < lsh_num_builtins_args(); i++) {
        printf("  %s\n", builtin_str_args[i]);
    }
    for (int i = 0; i < lsh_num_builtins_noargs(); i++) {
        printf("  %s\n", builtin_str_noargs[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int lsh_exit() { return 0; }
