#include <stddef.h>
#include <string.h>
#include "history.h"
#include <stdlib.h>
#include <stdio.h>

#define MAX_HISTORY_ENTRIES 100

struct historyEntry {
    int number;
    char *command;
};

static struct historyEntry history[MAX_HISTORY_ENTRIES];
static int history_count = 0;
static int history_limit = MAX_HISTORY_ENTRIES;
static int command_number = 0;
// keep track of low and high index so that if it goes over 100, we can still get the command 
// for example when we add the 101th elem, low index should become 1 and high index should become 100
// need to increment accordingly 

void hist_init(unsigned int max_limit)
{
    // TODO: set up history data structures, with 'limit' being the maximum
    // number of entries maintained.
    history_count = 0;
    history_limit = max_limit;

}
// frees the entire history
void hist_destroy(void)
{
    for(int i = 0; i < history_count; i++) {
        free(history[i].command);
    }
}

// add new entry to the end of the list
void hist_add(const char *cmd)
{
    const char *new_entry = cmd;
    if (new_entry == NULL) {
        return;
    }
    if (history_count >= history_limit) { // need to make room
        free(history[0].command); // free oldest 
        for (int i = 1; i < history_count; i++) {
            history[i - 1] = history[i]; // shift everything down one spot
        }
        history_count--;
    }
    history[history_count].command = strdup(new_entry);
    history[history_count++].number = ++command_number;
}

void hist_print(void)
{
    if (history_count == 0) {
        return;
    }
    for (int i = 0; i < history_count; i++) {
        printf("%d %s\n", history[i].number, history[i].command);
    }
    fflush(stdout);
}

const char *hist_search_prefix(char *prefix)
{   
    // TODO: Retrieves the most recent command starting with 'prefix', or NULL
    // if no match found.
    if (prefix == NULL || strlen(prefix) == 0) {
        return NULL;
    }
    int prefix_len = strlen(prefix);
    for (int i = history_count - 1; i >= 0; i--) { // start from the back because we want most recent
        if (strncmp(prefix, history[i].command, prefix_len) == 0) { // if the first prefix_len characters are equal we return it
            return history[i].command;
        }
    }
    return NULL;
}

const char *hist_search_cnum(int command_number)
{
    // TODO: Retrieves a particular command number. Return NULL if no match
    // found.
    int index = command_number - history[0].number;
    if (index >= 0 && index < history_count) {
        return history[index].command; // return command
    }
    return NULL;
}

unsigned int hist_last_cnum(void)
{
    // TODO: Retrieve the most recent command number.
    if (history_count == 0) {
        return 0;
    }
    return history[history_count - 1].number;
}
