#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "tokens.h"
#include <fcntl.h>
#include <errno.h>

int MAX_LINE = 255;
  
// Execute the given command if possible
void execute(char *command[], int command_index, char **previous_command) {
  // Explain the built in commands
  if (strcmp(command[0], "help") == 0) {
      printf("Built-in Commands:\ncd    change the current working directory of the shell\nsource       execute a given script\nprev    print the previous command line and execute it again\nhelp      explains all the built-in commands\n");
  }

  // Change the current working directory
  else if (strcmp(command[0], "cd") == 0) {
    chdir(command[1]);
  }

  // Print out and execute the previous command
  else if (strcmp(command[0], "prev") == 0) {
    printf("Previous Command: ");
    int i = 0;
    while (previous_command[i] != NULL) {
      printf("%s ", previous_command[i]);
      command[i] = previous_command[i];
      ++i;
    }
    printf("\n");
    char *redo_command[MAX_LINE];
    char **prev = get_tokens("prev");
    parse(redo_command, 0, command, 0, prev, i);
  }

  // Execute each line of the given file
  else if (strcmp(command[0], "source") == 0) {
    FILE *fd;
    fd = fopen(command[1], "r");
    char line[MAX_LINE];
    while(fgets(line, MAX_LINE, fd)) {
      char **tokenize = get_tokens(line);
      assert(tokenize != NULL);
      char **tokens = tokenize;
      int size = 0;
      while (tokens[size] != NULL) {
        size++;
      }
      parse(command, 0, tokens, 0, previous_command, size);
    }
    fclose(fd);
  }   

  // Exit the shell
  else if (strcmp(command[0], "exit") == 0) {
    printf("Bye bye.\n");
    exit(0);
  }

  // Else execute the command with exec
  else {
    command[command_index] = NULL;
    pid_t child = fork();
    if (child == 0) {
      if (execvp(command[0], command) == -1) {
        printf("%s: command not found\n", command[0]);
        exit(1);
      } 
    }
    else if (child == -1) {
      perror("Error- fork failed");
      exit(1);
    }
    wait(NULL);
  }
}


// Open a file to direct its input to the given command if possible
void inputredirect(char *command[], int command_index, char **tokens, int token_index) {
  command[command_index] = NULL;
  pid_t child = fork();
  if (child == 0) {
    if (close(0) == -1) {
      perror("Error closing stdin");
      exit(1);
    }
    int fd = open(tokens[token_index + 1], O_RDONLY);
    assert(fd == 0);
    if (execvp(command[0], command) == -1) {
      printf("%s: command not found\n", command[0]);
      exit(1);
    }
  }
  else if (child == -1) {
    perror("Error - fork failed");
    exit(1);
  }
  wait(NULL);
}  

// Open a file and direct the output of the previous command into the file
void outputredirect(char *command[], int command_index, char **tokens, int token_index) {
  command[command_index] = NULL;
  pid_t child = fork();
  if (child == 0) {
    if (close(1) == -1) {
      perror("Error closing stdout");
      exit(1);
    }
    int fd = open(tokens[token_index + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    assert(fd == 1);
    if (execvp(command[0], command) == -1) {
      printf("%s: command not found\n", command[0]);
      exit(1);
    }             
  }
  else if (child == -1) {
    perror("Error - fork failed");
    exit(1);
  }
  wait(NULL);
}

// Pipe commands together
void piping(char *command[], int command_index, char **tokens, int token_index, char **previous_command, int size) {
  // Parse commands
  // Get index of last pipe in input
  int last_pipe;
  int i = 0;
  while (tokens[i] != NULL) {
    if (strcmp(tokens[i], "|") == 0) {
      last_pipe = i;
    }
    ++i;
  }
  // Parse the first command of the pipe 
  char *first_command[255];
  int fc_index = 0;
  while (fc_index < last_pipe) {
    first_command[fc_index] = tokens[fc_index];
    ++fc_index;
  }
  // Parse the second command of the pipe
  char *second_command[255];
  int sc_index = 0;
  ++last_pipe;
  while (last_pipe < size) {
    second_command[sc_index] = tokens[last_pipe];
    ++last_pipe;
    ++sc_index;
  }
  first_command[fc_index] = NULL;
  second_command[sc_index] = NULL;
  
  // Get size of first command
  int size1 = fc_index;
 
  // Get size of second command
  i = 0;
  while(second_command[i] != NULL) {
    ++i;
  }
  int size2 = i;
  
  char *new_command[MAX_LINE];
  //Fork child A
  int child1 = fork();
  if (child1 == 0) {
    // In child A create a pipe and fork child B    
    int pipe_fds[2];
    assert(pipe(pipe_fds) == 0);
    int read_fd = pipe_fds[0];
    int write_fd = pipe_fds[1];
    pid_t child2 = fork();
    if (child2 == 0) {
      // In child B hook pipe to stdout, close other side, and execute first command
      close(read_fd);
      if (close(1) == -1) {
        perror("Error closing stdout");
        exit(1);
      }
      assert(dup(write_fd) == 1);
      parse(new_command, 0, first_command, 0, previous_command, size1);
      wait(NULL);
      exit(0);
    }
    // In child A hook pipe to stdin, close other side, and execute second command
    close(write_fd);
    if (close(0) == -1) {
      perror("Error closing stdin");
      exit(1);
    }
    assert(dup(read_fd) == 0);
    parse(new_command, 0, second_command, 0, previous_command, size2);
    wait(NULL);
    exit(0);
    }
 
    else if (child1 > 0) {
      wait(NULL);
    }
}


// Parse the user input and call the appropriate execution functions
void parse(char *command[], int command_index, char **tokens, int token_index, char **previous_command, int size) {
  // Iterate through the tokens
  while (token_index < size) {
    // Execute each line of the given file as a command
    if (strcmp(*tokens, "source") == 0) {
    }
    
    // If input has a sequence, execute the previous command and reset array
    if (strcmp(tokens[token_index], ";") == 0) {
      execute(command, command_index, previous_command);
      ++token_index;
      command_index = 0;
    }

    // If input has input redirection, open the given file and pass it to the command
    if (strcmp(tokens[token_index], "<") == 0) {
      inputredirect(command, command_index, tokens, token_index);
      token_index += 2;
      command_index = 0;
      continue;
    }

    // If input has output redirection, return the command output to given file
    if (strcmp(tokens[token_index], ">") == 0) {
      outputredirect(command, command_index, tokens, token_index);
      token_index += 2;
      command_index = 0;
      continue;
    }

    // If input has a pipe, execute the previous command and reset array
    if (strcmp(tokens[token_index], "|") == 0) {
      piping(command, command_index, tokens, token_index, previous_command, size);
      token_index = size; 
      command_index = 0;
      continue;
    }

    // Add token to array to store command
    else {
      char *copy2 = malloc((strlen(tokens[token_index]) + 1) * sizeof(char));
      strcpy(copy2, tokens[token_index]);
      command[command_index] = copy2;
      ++token_index;
      ++command_index;
    }
  }
  
  // Execute the last command in the array
  if (command[0] != NULL) {
    execute(command, command_index, previous_command);
  }
}


// Main function
int main(int argc, char **argv) {
  
  // Print the welcome message
  printf("Welcome to mini-shell.\n");
 
  while (1) {
    
    // Print the prompt and receive user input
    printf("shell $ ");
    char input[MAX_LINE];
    if (feof(stdin)) {
      printf("\nBye bye.\n");
      exit(0);
    }

    // Get input of user
    fgets(input, MAX_LINE, stdin);
    
    //If enter key is pressed, prompt user again
    if (input[0] == 0x0A) {
      continue;
    }

    // Get an array of token strings 
    char **tokenize = get_tokens(input);
    assert(tokenize != NULL);
    char **tokens = tokenize;
    char **previous_command;
    if (!strcmp(tokens[0], "prev") == 0) {
      previous_command = tokenize;
    }
    
    // Get size of token input
    int i = 0;
    while (tokens[i] != NULL) {
      ++i; 
    } 
    int size = i;

    // Create an array to store commands
    char *command[MAX_LINE];
    int command_index = 0;
    int token_index = 0;

    parse(command, command_index, tokens, token_index, previous_command, size);
  } 
  return 0;
}

