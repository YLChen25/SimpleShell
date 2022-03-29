// ############################## INCLUDE SECTION ######################################
#include <stdio.h>  // printf(), fgets()
#include <string.h> // strtok(), strcmp(), strdup()
#include <stdlib.h> // free()
#include <unistd.h> // fork()
#include <sys/types.h>
#include <sys/wait.h> // waitpid()
#include <sys/stat.h>
#include <fcntl.h> // open(), creat(), close()
#include <time.h>
#include <errno.h>
// ######################################################################################

// ############################## DEFINE SECTION ########################################
#define MAX_LINE_LENGTH 1024
#define BUFFER_SIZE 64
// RM
#define REDIR_SIZE 2

#define MAX_HISTORY_SIZE 128
#define MAX_COMMAND_NAME_LENGTH 128

#define PROMPT_FORMAT "%F %T "
#define PROMPT_MAX_LENGTH 30

// ######################################################################################


// ############################## GLOBAL VARIABLES SECTION ##############################
int running = 1;

// ######################################################################################


/**
 * Display the banner of the simple shell
 * @param None
 * @return None
 */
void init_shell() {
    printf("**********************************************************************\n");
    printf("  #####                                    #####                              \n");
    printf(" #     # # #    # #####  #      ######    #     # #    # ###### #      #      \n");
    printf(" #       # ##  ## #    # #      #         #       #    # #      #      #      \n");
    printf("  #####  # # ## # #    # #      #####      #####  ###### #####  #      #      \n");
    printf("       # # #    # #####  #      #               # #    # #      #      #      \n");
    printf(" #     # # #    # #      #      #         #     # #    # #      #      #      \n");
    printf("  #####  # #    # #      ###### ######     #####  #    # ###### ###### ###### \n");
    printf("**********************************************************************\n");
    char *username = getenv("USER");
    printf("\n\n\nCurrent user: @%s", username);
    printf("\n");
}

char *get_current_dir() {
    char cwd[FILENAME_MAX];
    char*result = getcwd(cwd, sizeof(cwd));
    return result;
}

/**
 * The shell prompt constructor has the form YYYY-MM-dd <space> hour:minute:second <space> default name of shell <space> >
 * @param None
 * @return a prompt string
 */
char *prompt() {
    static char *_prompt = NULL;
    time_t now;
    struct tm *tmp;
    size_t size;

    if (_prompt == NULL) {
        _prompt = malloc(PROMPT_MAX_LENGTH * sizeof(char));
        if (_prompt == NULL) {
            perror("Error: Unable to locate memory");
            exit(EXIT_FAILURE);
        }
    }

    // Get the date month year
    now = time(NULL);
    if (now == -1) {
        fprintf(stderr, "Error: Cannot get current timestamp");
        exit(EXIT_FAILURE);
    }

    // Get system time
    tmp = localtime(&now);
    if (tmp == NULL) {
        fprintf(stderr, "Error: Cannot identify timestamp");
        exit(EXIT_FAILURE);
    }

    // Create a string in the format YYYY-MM-dd <space> hour:minute:second <space>
    size = strftime(_prompt, PROMPT_MAX_LENGTH, PROMPT_FORMAT, tmp);
    if (size == 0) {
        fprintf(stderr, "Error: Cannot convert time to string");
        exit(EXIT_FAILURE);
    }
    // Add after the default shell name
    char* username = getenv("USER");
    strncat(_prompt, username, strlen(username));
    return _prompt;
}

/**
 * Error function
 * @param None
 * @return None
 */
void error_alert(char *msg) {
    printf("%s %s\n", prompt(), msg);
}

/**
 * @description: Function to remove carriage return from a string
 * @param: line is a string of characters
 * @return: Returns a string with carriage returns removed '\n'
 */
void remove_end_of_line(char *line) {
    int i = 0;
    while (line[i] != '\n') {
        i++;
    }

    line[i] = '\0';
}

// Readline
/**
 * @description: Function to read string input from keyboard
 * @param: line is a string of characters that stores the user input string
 * @return: none
 */
void read_line(char *line) {
    char *ret = fgets(line, MAX_LINE_LENGTH, stdin);

    // Format string: remove carriage return and mark '\n' with '\0' - end of string
    remove_end_of_line(line);

    // If the comparison shows that the input string is "exit" or "quit" or NULL, the program ends
    if (strcmp(line, "exit") == 0 || ret == NULL || strcmp(line, "quit") == 0) {
        exit(EXIT_SUCCESS);
    }
}

// Parser

/**
 * @description: The function parses the input string from the user and outputs the arguments
 * @param : input_string is the user input string, argv the string array contains the arg strings, is_background indicates whether the command is running in the background or not?
 * @return: none
 */
void parse_command(char *input_string, char **argv, int *wait) {
    int i = 0;

    while (i < BUFFER_SIZE) {
        argv[i] = NULL;
        i++;
    }

    // If - else 
    *wait = (input_string[strlen(input_string) - 1] == '&') ? 0 : 1; // If there is and then wait = 0, else wait = 1
    input_string[strlen(input_string) - 1] = (*wait == 0) ? input_string[strlen(input_string) - 1] = '\0' : input_string[strlen(input_string) - 1];
    i = 0;
    argv[i] = strtok(input_string, " ");

    if (argv[i] == NULL) return;

    while (argv[i] != NULL) {
        i++;
        argv[i] = strtok(NULL, " ");
    }

    argv[i] = NULL;
}

/**
 * @description 
 * @param 
 * @return
 */
void exec_parent(pid_t child_pid, int *bg) {}

// History
/**
 * @description Function to write previous command
 * @param history history string, line contains previous command
 * @return none
 */
void set_prev_command(char *history, char *line) {
    strcpy(history, line);
}

/**
 * @description Function to get the previous command
 * @param history Array of history
 * @return none
 */
char *get_prev_command(char *history) {
    if (history[0] == '\0') {
        fprintf(stderr, "No commands in history\n");
        return NULL;
    }
    return history;
}

// Built-in: Implement builtin functions
/*
  Function Declarations for builtin shell commands:
 */
int simple_shell_cd(char **args);
int simple_shell_help(char **args);
int simple_shell_exit(char **args);
void exec_command(char **args, char **redir_argv, int wait, int res);

// List of builtin commands
char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

// Corresponding functions.
int (*builtin_func[])(char **) = {
    &simple_shell_cd,
    &simple_shell_help,
    &simple_shell_exit
};

int simple_shell_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

// Implement - Setting

/**
 * @description CD function 
 * @param argv
 * @return 0
 */
int simple_shell_cd(char **argv) {
    if (argv[1] == NULL) {
        fprintf(stderr, "Error: Expected argument to \"cd\"\n");
    } else {
        // Change the process's working directory to PATH.
        if (chdir(argv[1]) != 0) {
            perror("Error: Error when change the process's working directory to PATH.");
        }
    }
    return 1;
}

/**
 * @description Help function prints the instructions of the shell
 * @param argv 
 * @return
 */
int simple_shell_help(char **argv) {
    static char help_team_information[] =
        "OPERATING SYSTEMS PROJECT 02 - A SIMPLE SHELL\n"
        
        "λ Team member λ\n"
        "110054835 \t\tYu-Jung Chen\n"
        "110054817 \t\tHui Huang\n"
        "107440020 \t\tHank Yang\n"
        "110598111 \t\tMark Hsieh\n"


        "λ Description λ\n"
        "This program was written in C as assignment for homework 01 in Operating Systems Course."
        "\n"
        
        
        "\nUsage help command. Type help [command name] for help/ more information.\n"
        "Options for [command name]:\n"
        "cd <directory name>\t\t\tDescription: Change the current working directory.\n"
        "exit              \t\t\tDescription: Exit simple shell.\n";
    static char help_cd_command[] = "HELP CD COMMAND\n";
    static char help_exit_command[] = "HELP EXIT COMMAND\n";

    if (strcmp(argv[0], "help") == 0 && argv[1] == NULL) {
        printf("%s", help_team_information);
        return 0;
    }

    if (strcmp(argv[1], "cd") == 0) {
        printf("%s", help_cd_command);
    } else if (strcmp(argv[1], "exit") == 0) {
        printf("%s", help_exit_command);
    } else {
        printf("%s", "Error: Too much arguments.");
        return 1;
    }
    return 0;
}

/**
 * @description Exit function
 * @param args String array containing the args to execute the command
 * @return
 */
int simple_shell_exit(char **args) {
    running = 0;
    return running;
}

/**
 * @description History display function
 * @param 
 * @return
 */
int simple_shell_history(char *history, char **redir_args) {
    char *cur_args[BUFFER_SIZE];
    char cur_command[MAX_LINE_LENGTH];
    int t_wait;

    if (history[0] == '\0') {
        fprintf(stderr, "No commands in history\n");
        return 1;
    }
    strcpy(cur_command, history);
    printf("%s\n", cur_command);
    parse_command(cur_command, cur_args, &t_wait);
    int res = 0;
    exec_command(cur_args, redir_args, t_wait, res);
    return res;
}

/**
 * @description Command execution function
 * @param 
 * @return
 */
void exec_command(char **args, char **redir_argv, int wait, int res) {
    // Check if there is a match in the builtin command array, if yes, then execute, if not, go to the next
    for (int i = 0; i < simple_shell_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            (*builtin_func[i])(args);
            res = 1;
        }
    }

    if (res == 0) {
        int status;

        // Create child process
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            execvp(args[0], args);
            exit(EXIT_SUCCESS);

        } else if (pid < 0) { // When child process creation fails
            perror("Error: Error forking");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            // printf("[LOGGING] Parent pid = <%d> spawned a child pid = <%d>.\n", getpid(), pid);
            waitpid(pid, &status, WUNTRACED);
        }
    }
}

/**
 * @description Main function
 * @param void 
 * @return 0 
 */
int main(void) {
    // Array to store args
    char *args[BUFFER_SIZE];

    // Command line
    char line[MAX_LINE_LENGTH];

    // String copied from line
    char t_line[MAX_LINE_LENGTH];

    // Array to store history
    char history[MAX_LINE_LENGTH] = "No commands in history";

    // Array containing agrs to perform IO . redirection
    // RM
    char *redir_argv[REDIR_SIZE];

    // Check if it's running in the background
    int wait;

    // Initialize banner shell
    init_shell();
    int res = 0;

    // Initialize an infinite loop
    while (running) {
        printf("%s:%s> ", prompt(), get_current_dir());
        fflush(stdout);

        // Read input string from user
        read_line(line);

        // Copy string
        strcpy(t_line, line);

        // Parser input string
        parse_command(line, args, &wait);

        // Command execution
        if (strcmp(args[0], "!!") == 0) {
            res = simple_shell_history(history, redir_argv);
        } else {
            set_prev_command(history, t_line);
            exec_command(args, redir_argv, wait, res);
        }
        res = 0;
    }
    return 0;
}