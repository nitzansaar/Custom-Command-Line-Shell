# Project 3: Command Line Shell
This project is a custom shell implementation designed to provide an interactive command-line interface for executing commands, managing I/O redirection, piping, and maintaining command history. It allows users to enter commands at a prompt, which are then parsed into tokens and executed. The shell handles basic commands and offers the ability to search and recall previous commands through its command history functionality. The shell is built around a main loop that continuously reads user input, tokenizes it, and processes commands accordingly. It supports I/O redirection using the '<', '>', and '>>' operators, thus allowing users to direct the input and output of commands to and from files. Additionally, when the user types 'exit, the shell prints a farewell message with a random emoji and an inspirational quote before closing.

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
[nesaar@nesaar-vm P3-nitzansaar]$ ./digitaldash
[😢 nesaar 2023-04-26 23:21:24 | Total Commands: 0] > ls -1 -- / | sort -r
shell.c:236:main(): input command: ls -1 -- / | sort -r
shell.c:111:execute_pipeline(): exec: ls
var
usr
tmp
sys
srv
sbin
run
root
proc
opt
mnt
lib64
lib
home
etc
dev
bootstrap.sh
boot
bin
[😊 nesaar 2023-04-26 23:21:27 | Total Commands: 1] > echo -----
shell.c:236:main(): input command: echo -----
shell.c:111:execute_pipeline(): exec: echo
-----
[👐 nesaar 2023-04-26 23:21:36 | Total Commands: 2] > seq 10000000 | wc -l
shell.c:236:main(): input command: seq 10000000 | wc -l
shell.c:111:execute_pipeline(): exec: seq
10000000
[👻 nesaar 2023-04-26 23:21:45 | Total Commands: 3] > exit
shell.c:236:main(): input command: exit
🤣Thanks for using Digital DaSH! Have a great day!
🤣You miss 100% of the shots you don't take. - Wayne Gretzky

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
