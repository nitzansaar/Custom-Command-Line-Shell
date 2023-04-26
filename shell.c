#include <fcntl.h>
#include <limits.h>
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
#include <signal.h>

struct command_line {
    char **tokens;
    bool stdout_pipe;
    char *stdout_file;
    char *stdin_file;
    bool stdout_append;;
};

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
    size_t tok_start = strspn(*str_ptr, delim); // the length of *str_ptr that contains only characters from delim
    size_t tok_end = strcspn(*str_ptr + tok_start, delim); // the length of *str_ptr + tok_start that does not contain any characters from delim
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
void execute_pipeline(struct command_line *cmds)
{
    LOG("exec: %s\n", cmds[0].tokens[0]);
    int i = 0;
    while (cmds[i].stdout_pipe == true) { // needs to run for each command
        int fd[2];
        pipe(fd);
        pid_t pid = fork();
        if (pid == 0) { // children
            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]);
            execvp(cmds[i].tokens[0], cmds[i].tokens);
        } else { // parent
            dup2(fd[0], STDIN_FILENO);
            close(fd[1]);
        }
        i++;
    }

    if (cmds[i].stdout_append) {
        int output = open(cmds[i].stdout_file, O_CREAT | O_WRONLY | O_APPEND, 0666);
        dup2(output, STDOUT_FILENO);
        close(output);
    } else if (cmds[i].stdout_file) {
        int output = open(cmds[i].stdout_file, O_CREAT | O_WRONLY | O_TRUNC, 0666);
        dup2(output, STDOUT_FILENO);
        close(output);
    }

    if (cmds[i].stdin_file) {
        int input = open(cmds[i].stdin_file, O_RDONLY);
        dup2(input, STDIN_FILENO);
        close(input);
    }

    execvp(cmds[i].tokens[0], cmds[i].tokens);
}


char *read_script(void) {
    char *line_ptr = NULL; // buffer(temporary storage) for storing the line
    size_t line_sz = 0;
    ssize_t res = getline(&line_ptr, &line_sz, stdin); // reads until newline character or end of stream
    if (res == -1) {
        free(line_ptr);
        perror("getline");
        return NULL;
    }
    char *hash_pos = strchr(line_ptr, '#'); // don't want comments to be executed
    if (hash_pos != NULL) {
        *hash_pos = '\0'; // set to nullbyte
    } else {
        free(hash_pos);
    }
    line_ptr[res - 1] = '\0'; // set last character to nullbyte
    return line_ptr;
}

int main(void)
{
    // NOTE: "scripting" mode really just means reading from stdin
    //       and NOT printing a whole bunch of junk (including the prompt)
    signal(SIGINT, SIG_IGN);
    rl_startup_hook = readline_init;
    hist_init(100);
    char *command; //need to check isatty() if a person -> allows to type if not just reads command
    while (true) 
    {
        if (isatty(STDIN_FILENO)) {
            char *prompt =
            //  "     / \\__\n"
            //                 "    (    @\\___\n"
            //                 "     /         O\n"
            //                 "    /   (_____/\n"
            //                 "   /_____/   U\n";
        //                     prompt =
        "  /\\_/\\\n"
        " ( o.o )\n"
        "   >^<\n";

        // "      \":\"\n    ___:____     |\"\\/\"|\n  ,'        `.    \\  /\n  |  O        \\___/  |\n~^~^~^~^~^~^~^~^~^~^~^~^~\n""🌊 Enter your command below! 🌊\n";
        //  "┈┈╱▔▔▔▔▔▔▔▔▔▔▔▏\n┈╱╭▏╮╭┻┻╮╭┻┻╮╭▏\n▕╮╰▏╯┃╭╮┃┃╭╮┃╰▏\n▕╯┈▏┈┗┻┻┛┗┻┻┻╮▏\n▕╭╮▏╮┈┈┈┈┏━━━╯▏\n▕╰╯▏╯╰┳┳┳┳┳┳╯╭▏\n▕┈╭▏╭╮┃┗┛┗┛┃┈╰▏\n▕┈╰▏╰╯╰━━━━╯┈┈▏\n";
        
            command = readline(prompt);
        }else {
            command = read_script();
        }
        if (command == NULL) {
            break;
        }
        if ('!' == command[0]) {
            if (isdigit(command[1])) {
                const char *res = hist_search_cnum(strtol(command + 1, NULL, 10));
                if (res != NULL) {
                    free(command);
                    command = strdup(res);
                } else {
                    continue; 
                }
            } else if (isalpha(command[1])) {
                const char *res = hist_search_prefix(command+1);
                if (res != NULL) {
                    free(command);
                    command = strdup(res);
                } else {
                    continue; 
                }
            } else if (command[1] == '!') {
                int last_cnum = hist_last_cnum(); 
                const char *res = hist_search_cnum(last_cnum);
                if (res != NULL) {
                    free(command);
                    command = strdup(res);
                } else {
                    continue; 
                }
            }
        }
        LOG("input command: %s\n", command);
        char *args[1024] = {0};
        int tokens = 0;
        char *next_tok = command;
        char *curr_tok; 
        char *og_tok = strdup(command); // need to save pointer to original token because command gets modified
        // tokenize command. ex: ls -l / => 'ls', 'l', '/', 'NULL'
        while ((curr_tok = next_token(&next_tok, " \t\r\n")) != NULL) // handle for pipes
        {
		    args[tokens++] = curr_tok;
	    }
	    args[tokens] = NULL;
        struct command_line cmd_list[1024] = { 0 };
        cmd_list[0].tokens = &args[0]; // set first command to mem address of the first token in case there is no pipes
	    int commands = 0;

	for (int i = 0; i < tokens; i++) {
        if (strcmp(args[i], "|") == 0) {
            args[i] = NULL;
            cmd_list[commands].stdout_pipe = true;
            cmd_list[commands + 1].tokens = &args[i + 1];
            commands++;
        } else if (strcmp(args[i], "<") == 0) {
            args[i] = NULL;
            cmd_list[commands].stdin_file = args[i + 1];
            i++; // skip the file name in the next iteration
        } else if (strcmp(args[i], ">") == 0) {
            args[i] = NULL;
            cmd_list[commands].stdout_file = args[i + 1];
            // cmd_list[commands].stdout_pipe = false; // set to false for overwriting
            i++; // skip the file name in the next iteration
        } else if (strcmp(args[i], ">>") == 0) {
            args[i] = NULL;
            cmd_list[commands].stdout_file = args[i + 1];
            cmd_list[commands].stdout_pipe = false;
            cmd_list[commands].stdout_append = true;
            i++; // Skip the file name in the next iteration
        }
	}

        if (args[0] == (char *) NULL) { //blank command
            continue;
        }
        if (strcmp(args[0], "exit") == 0) {
            fprintf(stderr, "Have a great day, bye!\n");
            break;
        }
        // chdir system call
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                chdir(getenv("HOME")); // go to home directory if unspecified
            } else {
                chdir(args[1]); // go to specified directory, i think it isn't working
            }
        }
        // LOG("og command: %s\n", og_tok);
        hist_add(og_tok);
        // history --- want to print after we added all tokens
        if (strcmp(args[0], "history") == 0) {
            hist_print();
        }

        pid_t child = fork();
        if (child == -1) {
            perror("fork");
        } else if (child == 0) { // child is always 0
            execute_pipeline(cmd_list);
            perror("exec");
            close(fileno(stdin));
            return EXIT_FAILURE;
        } else {
            int status;
            wait(&status); // what is going on here?
        }
        free(command);
    }
    hist_destroy();
    return 0;
}
