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
#include <time.h>

const char *emojis[] = {"😂", "🤣", "😅", "😆", "😜", "😝", "😛", "🤪", "🤨", "🙃",
    "🤓", "😎", "🥸", "🤩", "🥳", "😏", "😁", "😬", "🤭", "🤫",
    "🤔", "🤥", "🤯", "😵‍💫", "😵", "🥴", "😶‍🌫️", "😸", "😹", "😻",
    "🙀", "🤖", "👻", "💩", "🤡", "🥶", "🥵", "🤠", "🥺", "😺",
    "😼", "🤑", "🤢", "🤮", "🤧", "🙄", "😋", "😙", "😗", "🧐"};
const int emoji_count = sizeof(emojis) / sizeof(emojis[0]);
int command_count = 0;
const char *quotes[] = {
        "The greatest glory in living lies not in never falling, but in rising every time we fall. - Nelson Mandela",
        "The way to get started is to quit talking and begin doing. - Walt Disney",
        "You have to expect things of yourself before you can do them. - Michael Jordan",
        "The only way to do great work is to love what you do. - Steve Jobs",
        "You miss 100% of the shots you don't take. - Wayne Gretzky",
        "In the end, it's not the years in your life that count. It's the life in your years. - Abraham Lincoln",
        "Impossible is just a big word thrown around by small men who find it easier to live in the world they've been given than to explore the power they have to change it. - Muhammad Ali",
        "The most important thing is to try and inspire people so that they can be great in whatever they want to do. - Kobe Bryant"
    };
const int quote_count = sizeof(quotes) / sizeof(quotes[0]);

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
const char *get_random_quote() {

    srand(time(NULL));
    int index = rand() % quote_count;
    return quotes[index];
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
    while (cmds[i].stdout_pipe) { // needs to run for each command
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
    } else if (cmds[i].stdout_file) { 
        int output = open(cmds[i].stdout_file, O_CREAT | O_WRONLY | O_TRUNC, 0666);
        dup2(output, STDOUT_FILENO);
    }

    if (cmds[i].stdin_file) { 
        int input = open(cmds[i].stdin_file, O_RDONLY);
        dup2(input, STDIN_FILENO);
    }

    execvp(cmds[i].tokens[0], cmds[i].tokens);
}

char *get_username() {
    struct passwd *pw = getpwuid(getuid());
    return pw->pw_name;
}


const char *get_random_emoji() {
    srand(time(NULL));
    int index = rand() % emoji_count;
    return emojis[index];
}

char *get_current_date_time() {
    time_t rawtime;
    struct tm *timeinfo;
    static char date_time_str[20];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(date_time_str, sizeof(date_time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
    return date_time_str;
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
    signal(SIGINT, SIG_IGN);
    rl_startup_hook = readline_init;
    hist_init(100);
    char *command;
    while (true) 
    {
        if (isatty(STDIN_FILENO)) {
            char prompt[PATH_MAX + 50];
            snprintf(prompt, sizeof(prompt), "[%s %s %s | Total Commands: %d] > ",
                                            get_random_emoji(), get_username(), get_current_date_time(), command_count++);
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
        while ((curr_tok = next_token(&next_tok, " \t\r\n")) != NULL)
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
                cmd_list[commands + 1].tokens = &args[i + 1]; // echo null ls
                commands++;
            } else if (strcmp(args[i], "<") == 0) {
                args[i] = NULL;
                cmd_list[commands].stdin_file = args[i + 1];// hello > file null
                i++; // skip the file name in the next iteration
            } else if (strcmp(args[i], ">") == 0) {
                args[i] = NULL;
                cmd_list[commands].stdout_file = args[i + 1];
                i++;
            } else if (strcmp(args[i], ">>") == 0) {
                args[i] = NULL;
                cmd_list[commands].stdout_file = args[i + 1];
                cmd_list[commands].stdout_pipe = false;
                cmd_list[commands].stdout_append = true;
                i++;
            }
        }

        if (args[0] == (char *) NULL) { //blank command
            free(command);
            free(og_tok);
            continue;
        }
        if (strcmp(args[0], "exit") == 0) {
            fprintf(stderr, "%sThanks for using Digital DaSH! Here is an inspirational quote:\n%s %s\n",
            get_random_emoji(), get_random_emoji(), get_random_quote());
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
        hist_add(og_tok);
        // history
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
            wait(&status); 
        }
        free(og_tok);
        free(command);
    }
    hist_destroy();
    return 0;
}