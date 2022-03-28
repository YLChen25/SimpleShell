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
 * Hàm khởi tạo banner cho shell
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
 * Hàm khởi tạo Shell Prompt có dạng YYYY-MM-dd <space> hour:minute:second <space> default name of shell <space> >
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

    // Lấy ngày tháng năm
    now = time(NULL);
    if (now == -1) {
        fprintf(stderr, "Error: Cannot get current timestamp");
        exit(EXIT_FAILURE);
    }

    // Lấy giờ hệ thống
    tmp = localtime(&now);
    if (tmp == NULL) {
        fprintf(stderr, "Error: Cannot identify timestamp");
        exit(EXIT_FAILURE);
    }

    // Tạo chuỗi theo format YYYY-MM-dd <space> hour:minute:second <space>
    size = strftime(_prompt, PROMPT_MAX_LENGTH, PROMPT_FORMAT, tmp);
    if (size == 0) {
        fprintf(stderr, "Error: Cannot convert time to string");
        exit(EXIT_FAILURE);
    }
    // Thêm vào sau tên mặc định của shell
    char* username = getenv("USER");
    strncat(_prompt, username, strlen(username));
    return _prompt;
}

/**
 * Hàm báo lỗi
 * @param None
 * @return None
 */
void error_alert(char *msg) {
    printf("%s %s\n", prompt(), msg);
}

/**
 * @description: Hàm xóa dấu xuống dòng của một chuỗi
 * @param: line là một chuỗi các ký tự
 * @return: trả về một chuỗi đã được xóa dấu xuống dòng '\n'
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
 * @description: Hàm đọc chuỗi nhập từ bàn phím 
 * @param: line là một chuỗi các ký tự lưu chuỗi người dùng nhập vào
 * @return: none
 */
void read_line(char *line) {
    char *ret = fgets(line, MAX_LINE_LENGTH, stdin);

    // Định dạng lại chuỗi: xóa ký tự xuống dòng và đánh dấu vị trí '\n' bằng '\0' - kết thúc chuỗi
    remove_end_of_line(line);

    // Nếu so sánh thấy chuỗi đầu vào là "exit" hoặc "quit" hoặc là NULL thì kết thúc chương trình
    if (strcmp(line, "exit") == 0 || ret == NULL || strcmp(line, "quit") == 0) {
        exit(EXIT_SUCCESS);
    }
}

// Parser

/**
 * @description: Hàm parse chuỗi input từ người dùng ra những argument
 * @param : input_string là chuỗi người dùng nhập vào, argv mảng chuỗi chứa những chuỗi arg, is_background cho biết lệnh có chạy nền hay không?
 * @return: none
 */
void parse_command(char *input_string, char **argv, int *wait) {
    int i = 0;

    while (i < BUFFER_SIZE) {
        argv[i] = NULL;
        i++;
    }

    // If - else cho gọn tí
    *wait = (input_string[strlen(input_string) - 1] == '&') ? 0 : 1; // Nếu có & thì wait = 0, ngược lại wait = 1
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
 * @description Hàm ghi lệnh trước đó
 * @param history chuỗi history, line chưa lệnh trước đó
 * @return none
 */
void set_prev_command(char *history, char *line) {
    strcpy(history, line);
}

/**
 * @description Hàm lấy lệnh trước đó
 * @param history chuỗi history
 * @return none
 */
char *get_prev_command(char *history) {
    if (history[0] == '\0') {
        fprintf(stderr, "No commands in history\n");
        return NULL;
    }
    return history;
}

// Built-in: Implement builtin functions để thực hiện vài lệnh cơ bản như cd (change directory), demo custome help command
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

// Implement - Cài đặt

/**
 * @description Hàm cd (change directory) bằng cách gọi hàm chdir()
 * @param argv mảng chuỗi chứa những chuỗi arg để thực hiện lệnh
 * @return 0 nếu thất bại, 1 nếu thành công
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
 * @description Hàm help in ra command những chuỗi hướng dẫn
 * @param argv mảng chuỗi chứa những chuỗi arg để thực hiện lệnh
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
 * @description Hàm thoát
 * @param args mảng chuỗi chứa những chuỗi arg để thực hiện lệnh
 * @return
 */
int simple_shell_exit(char **args) {
    running = 0;
    return running;
}

/**
 * @description Hàm thoát
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
 * @description Hàm thực thi lệnh
 * @param 
 * @return
 */
void exec_command(char **args, char **redir_argv, int wait, int res) {
    // Kiểm tra có trùng với lệnh nào trong mảng builtin command không, có thì thực thi, không thì xuống tiếp dưới
    for (int i = 0; i < simple_shell_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            (*builtin_func[i])(args);
            res = 1;
        }
    }

    // Chưa thực thi builtin commands
    if (res == 0) {
        int status;

        // Tạo tiến trình con
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            execvp(args[0], args);
            exit(EXIT_SUCCESS);

        } else if (pid < 0) { // Khi mà việc tạo tiến trình con bị lỗi
            perror("Error: Error forking");
            exit(EXIT_FAILURE);
        } else { // Thực thi chạy nền
            // Parent process
            // printf("[LOGGING] Parent pid = <%d> spawned a child pid = <%d>.\n", getpid(), pid);
            waitpid(pid, &status, WUNTRACED);
        }
    }
}

/**
 * @description Hàm main :))
 * @param void không có gì
 * @return 0 nếu hết chương trình
 */
int main(void) {
    // Mảng chưa các agrs
    char *args[BUFFER_SIZE];

    // Chuỗi line
    char line[MAX_LINE_LENGTH];

    // Chuỗi sao chép từ line
    char t_line[MAX_LINE_LENGTH];

    // Chuỗi lưu trữ lịch sử
    char history[MAX_LINE_LENGTH] = "No commands in history";

    // Mảng chứa agrs để thực tthi chuyển hướng IO
    // RM
    char *redir_argv[REDIR_SIZE];

    // Check xem có chạy nền không
    int wait;

    // Khởi tạo banner shell
    init_shell();
    int res = 0;

    // Khởi tạo một vòng lặp vô hạn
    while (running) {
        printf("%s:%s> ", prompt(), get_current_dir());
        fflush(stdout);

        // Đọc chuuỗi nhận vào từ người dùng
        read_line(line);

        // Sao chép chuỗi
        strcpy(t_line, line);

        // Parser chuỗi input
        parse_command(line, args, &wait);

        // Thực thi lệnh
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