#include <fcntl.h>
#include <locale.h>
#include <pwd.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "history.h"
#include "logger.h"

int readline_init(void)
{
    rl_variable_bind("show-all-if-ambiguous", "on");
    rl_variable_bind("colored-completion-prefix", "on");
    return 0;
}

int main(void)
{
    rl_startup_hook = readline_init;
   
    char *command;
    while (true) {
        command = readline("prompt> ");
        if (command == NULL) {
            break;
        }

        LOG("Input command: %s\n", command);

        /* We are done with command; free it */
        free(command);
    }

    return 0;
}
