#include <fcntl.h>
#include <locale.h>
#include <pwd.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <stddef.h>
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

/**
 * Retrieves the next token from a string.
 *
 * Parameters:
 * - str_ptr: maintains context in the string, i.e., where the next token in the
 *   string will be. If the function returns token N, then str_ptr will be
 *   updated to point to token N+1. To initialize, declare a char * that points
 *   to the string being tokenized. The pointer will be updated after each
 *   successive call to next_token.
 *
 * - delim: the set of characters to use as delimiters
 *
 * Returns: char pointer to the next token in the string.
 */
char *next_token(char **str_ptr, const char *delim)
{
    if (*str_ptr == NULL) {
        return NULL;
    }
    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);
    /* Zero length token. We must be finished. */
    if (tok_end  == 0) {
        *str_ptr = NULL;
        return NULL;
    }
    /* Take note of the start of the current token. We'll return it later. */
    char *current_ptr = *str_ptr + tok_start;
    /* Shift pointer forward (to the end of the current token) */
    *str_ptr += tok_start + tok_end;
    if (**str_ptr == '\0') {
        /* If the end of the current token is also the end of the string, we
         * must be at the last token. */
        *str_ptr = NULL;
    } else {
        /* Replace the matching delimiter with a NUL character to terminate the
         * token string. */
        **str_ptr = '\0';
        /* Shift forward one character over the newly-placed NUL so that
         * next_pointer now points at the first character of the next token. */
        (*str_ptr)++;
    }
    return current_ptr;
}

char *read_script(void) {
    char *line = NULL;
    size_t line_sz = 0;
    ssize_t res = getline(&line, &line_sz, stdin);
    if (res == -1) {
        free(line);
        perror("getline");
        return NULL;
    }
    char *hash_pos = strchr(line, '#'); // don't want comments to be executed
    if (hash_pos != NULL) {
        *hash_pos = '\0';
    } else {
        free(hash_pos);
    }
    line[res - 1] = '\0';
    return line;
}


// add ffliush(stdout); to the end of hist_print fxn
int main(void)
{
    // NOTE: "scripting" mode really just means reading from stdin
    //       and NOT printing a whole bunch of junk (including the prompt)
    // rl_startup_hook = readline_init;
    hist_init(100);
    char *command;
    while (true) 
    {
        command = read_script();
        if (command == NULL) {
            break;
        }
        // LOG("input command: %s\n", command);
        hist_add(command);

        char *args[20] = {0};
        int tokens = 0;
        char *next_tok = command;
        char *curr_tok; 
        // tokenize command. ex: ls -l / => 'ls', 'l', '/', 'NULL'
        while ((curr_tok = next_token(&next_tok, " \t\r\n")) != NULL)
        {
            args[tokens++] = curr_tok;
            LOG("Token %02d: '%s'\n", tokens, curr_tok);
        }
        args[tokens] = (char *) NULL;
        if (args[0] == (char *) NULL) {
            continue;
        }
        else if (strcmp(args[0], "exit") == 0) {
            fprintf(stderr, "Have a great day, bye!\n");
            break;
        }
        // chdir system call
        else if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                chdir(getenv("HOME"));
            } else {
                chdir(args[1]);
            }
        }
        // history
        else if (strcmp(args[0], "history") == 0) {
            hist_print();
        }

        pid_t child = fork();
        if (child == -1) {
            perror("fork");
        } else if (child == 0) {
            execvp(args[0], args); 
            perror("exec");
            close(fileno(stdin));
            return EXIT_FAILURE;
        } else {
            int status;
            wait(&status);
        }
        free(command);
    }
    hist_destroy();
    return 0;
}
