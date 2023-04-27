# Project 3: Command Line Shell
This project is a custom shell implementation designed to provide an interactive command-line interface for executing commands, managing I/O redirection, piping, and maintaining command history. It allows users to enter commands at a prompt, which are then parsed into tokens and executed.The shell handles basic commands and offers the ability to search and recall previous commands through its command history functionality. The shell is built around a main loop that continuously reads user input, tokenizes it, and processes commands accordingly. It supports I/O redirection using the '<', '>', and '>>' operators, thus allowing users to direct the input and output of commands to and from files. Some additional features I added are a custom prompt that includes a random emoji, the username, the current date and time, and the total number of commands executed so far. Additionally, when the user types 'exit, the shell prints a farewell message with a random emoji and an inspirational quote before closing.

The prompt has the following items:

* Item1: Random Emoji
* Item2: Username
* Item3: Current Date and Time
* Item4: Number of Commands Executed so far

And it looks like this:

PROMPT: [🤓 nesaar 2023-04-26 22:28:10 | Total Commands: 0] > 

## Building
To compile and run:

```bash
make
./digitaldash
```

## Running + Example Usage

add a screenshot / text of a demo run of your program here. 

## Testing

To execute the test cases, use `make test`. To pull in updated test cases, run `make testupdate`. You can also run a specific test case instead of all of them:

```
# Run all test cases:
make test

# Run a specific test case:
make test run=4

# Run a few specific test cases (4, 8, and 12 in this case):
make test run='4 8 12'

# Run a test case in gdb:
make test run=4 debug=on
```
