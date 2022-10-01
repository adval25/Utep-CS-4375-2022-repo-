#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_LEN (size_t) 4096
#define FIRST_LINE_SIZE (size_t) 2
#define FIRST_ARRAY_SIZE (size_t) 2

void free_mem(char **lines, size_t lines_len, char *size_of_lines) {
  size_t i;
  for (i = (size_t) 0; i < lines_len; i++) {
    free(lines[i]);
  }
  free(lines);
  free(size_of_lines);
}

int my_write(int fd, const char *buf, size_t bytes) {
  size_t bytes_to_be_written;
  size_t bytes_already_written;
  ssize_t bytes_written_this_time;

  bytes_to_be_written = bytes;
  bytes_already_written = (size_t) 0;
  
  while (bytes_to_be_written > ((size_t) 0)) {
    bytes_written_this_time = write(fd, &buf[bytes_already_written], bytes_to_be_written);
    if (bytes_written_this_time < ((ssize_t) 0)) {
      return -1;
    }
    bytes_to_be_written -= (size_t) bytes_written_this_time;
    bytes_already_written += (size_t) bytes_written_this_time;
  }
  return 0;
}

int my_atoi(char *string_n) {
  int i, num;
  num = 0;

  for (i = 0; string_n[i] >= '0' && string_n[i] <= '9'; i++) {
    // if you make it inside for loop, you are an integer
    //num = num == -1 ? 0 : -1;
    num = (num * 10) + (string_n[i] - '0');
  }
  // return -1 if string is not integer, otherwise return integer
  return num;
}

int check_argv(char *argument) {
  // checks if argument matches "-n" prefix
  if (argument[0] == '-' && argument[1] == 'n' && argument[2] == 0) {
    return 0;
  }
  return 1;
}

int main(int argc, char** argv) {
  char buffer[BUFFER_LEN];
  ssize_t read_res;
  size_t read_bytes;
  int num_lines;
  int fd;
  
  // initialize variables
  num_lines = 0; // number of lines to be printed by head
  fd = 0; // file descriptor intialized to standard input (stdin)
  
  // check arguments are valid
  if (argc == 1) { // no option, no filename: copy first 10 lines of stdin
    num_lines = 10;
  }
  else if (argc == 3) { // -n [num] is specified (ideally), otherwise error
    num_lines = my_atoi(argv[2]);
    if (check_argv(argv[1]) == 1 || num_lines == -1) {
      return 1;
    }
  }
  else if (argc == 4) { // -n [num] filename / filename -n [num] specified, otherwise error
    if (check_argv(argv[1]) != 1) {
      num_lines = my_atoi(argv[2]);
      if (num_lines >= 0) {
	fd = open(argv[3], O_RDONLY);
	// error opening/reading file
	if (fd == -1) {
	  printf("Error Opening File.\n");
	  close(fd);
	  return 1;
	}
      }
    }
    else if(check_argv(argv[2]) != 1) {
      num_lines = my_atoi(argv[3]);
      if (num_lines >= 0) {
	fd = open(argv[1], O_RDONLY);
	// error opening/reading file
	if (fd == -1) {
	  printf("Error Opening File.\n");
	  close(fd);
	  return 1;
	}
      }
    } else {
      // -n [num] is missing and replaced w/ invalid parameter
      return 1;
    }
  } else {
    return 1;
  }

  // reading from standard input (keyboard)
  if (fd == 0) {
    while (num_lines > 0) {   
      // read input into buffer
      read_res = read(fd, buffer, sizeof(buffer));
		
      // returned value of 0 indicates an EOF--break from while loop
      if (read_res == ((ssize_t) 0)) break;
		
      // returned value is negative, meaning there's an error--end the program
      if (read_res < ((size_t) 0)) {
	// display error
	fprintf(stderr, "Error reading: %s\n", strerror(errno));
	close(fd);
	return 1;
      }
		
      // no errors--we can set read_bytes value & continue w/ program
      read_bytes = (size_t) read_res;

      // write onto standard out
      if (my_write(1, buffer, read_bytes) < 0) {
	fprintf(stderr, "Error writing: %s\n", strerror(errno));
	close(fd);
	return 1;
      }
      // reduce line counter
      num_lines--;
    }
  } else { // reading from file
    // variables
    char *c_line; // current line
    char *ptr;
    char **lines;
    char **lptr;
    char *size_of_lines;
    char *solptr;
    size_t offset;
    size_t c_line_len;
    size_t c_line_size;
    size_t lines_len;
    size_t lines_size;
    int a = 3;
    size_t i;
    char c;

    offset = (size_t) 0;
    c_line = NULL;
    lines = NULL;
    size_of_lines = NULL;
    lines_len = (size_t) 0;
    lines_size = (size_t) 0;
    c_line_len = (size_t) 0;
    c_line_size = (size_t) 0;
    
    while (1) {
      // read from file
      read_res = read(fd, buffer, sizeof(buffer));
      
      // returned value of 0 indicates an EOF--break from while loop
      if (read_res == ((ssize_t) 0)) {
	break;
      }      
      
      // returned value is negative, meaning there's an error--end the program
      if (read_res < ((size_t) 0)) {
	// display error
	fprintf(stderr, "Error reading: %s\n", strerror(errno));

	// deallocate memory & close file descriptor
	free_mem(lines, lines_len, size_of_lines);
	close(fd);
	return 1;
      }

      // no errors--we can set read_bytes value & continue w/ program
      read_bytes = (size_t) read_res;

      // resize readbytes for memory allocation when a newline character is detected
      for (i = (size_t) 0; i < read_bytes; i++) {
	// current character in buffer
	c = buffer[i];

	// put the current character into the current line
	if ((c_line_len + ((size_t) 1)) > c_line_size) {
	  /* allocate memory */
	  if ((c_line_len == ((size_t) 0)) || (c_line == NULL)) {
	    // first allocation
	    ptr = (char *) malloc(FIRST_LINE_SIZE);
	    if (ptr == NULL) {
	      // deallocate memory & close file descriptor
	      free_mem(lines, lines_len, size_of_lines);
	      close(fd);
	      return 1;
	    }
	    c_line = ptr;
	    c_line_size = FIRST_LINE_SIZE;
	  } else {
	    // reallocation
	    ptr = (char *) realloc(c_line, c_line_size * ((size_t) 2));
	    if (ptr == NULL) {
	      // deallocate memory & close file descriptor
	      free_mem(lines, lines_len, size_of_lines);
	      close(fd);
	      return 1;
	    }
	    c_line = ptr;
	    c_line_size *= (size_t) 2;
	  }
	}
	c_line[c_line_len] = c;
	c_line_len++;	

	// newline character check; increase number of "lines"
	if (c == '\n') {
	  // allocate memory for lines array
	  if (lines_len + ((size_t) 1) > lines_size) {
	    if (lines_len == ((size_t) 0) || lines == NULL) {
	      // first allocation
	      lptr = (char **) malloc(FIRST_ARRAY_SIZE * sizeof(char *));
	      if (lptr == NULL) {
		// deallocate memory & close file descriptor
		free_mem(lines, lines_len, size_of_lines);
		close(fd);
		return 1;
	      }
	      lines = lptr;
	      lines_size = FIRST_ARRAY_SIZE;
	      solptr = (char *) malloc(FIRST_ARRAY_SIZE * sizeof(size_t));
	      if (solptr == NULL) {
		// deallocate memory & close file descriptor
		free_mem(lines, lines_len, size_of_lines);
		close(fd);
		return 1;
	      }
	      size_of_lines = solptr;
	    } else {
	      // reallocation
	      lptr = (char **) realloc(lines, lines_size * ((size_t) 2) * sizeof(char *));
	      if (lptr == NULL) {
		// deallocate memory & close file descriptor
		close(fd);
		free_mem(lines, lines_len, size_of_lines);
		return 1;
	      }
	      lines = lptr;
	      solptr = (char *) realloc(size_of_lines, lines_size * ((size_t) 2) * sizeof(size_t));
	      if (solptr == NULL) {
	        // deallocate memory & close file descriptor
		close(fd);
		free_mem(lines, lines_len, size_of_lines);
		return 1;
	      }
	      lines_size *= (size_t) 2;
	      size_of_lines = solptr;	  
	    }
	  }
	  lines[lines_len] = c_line;
	  size_of_lines[lines_len] = c_line_len;
	  lines_len++;
	  c_line = NULL;
	  c_line_len = (size_t) 0;
	  c_line_size = (size_t) 0;

	  // break for loop if lines exceed num_lines
	  if (lines_len >= (size_t) num_lines) {
	    break;
	  }
	}
      }

      // break while loop if lines exceed num_lines
      if (lines_len >= (size_t) num_lines) {
	break;
      }
    }
    
    // close the file descriptor
    close(fd);
    
    // write onto standard out
    for (i = (size_t) 0; i < lines_len; i++) {
      if (my_write(1, lines[i], size_of_lines[i]) < 0) {
	fprintf(stderr, "Error writing: %s\n", strerror(errno));
	return 1;
      }
    }

    // free/deallocate memory
    free_mem(lines, lines_len, size_of_lines);
  }
  // signal success
  return 0;
}
