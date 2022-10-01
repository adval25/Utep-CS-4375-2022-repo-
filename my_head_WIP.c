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
  if (argument[0] == '-' && argument[1] == 'n' && argument[2] == 0) {
    //printf("char: %d", argument[2]);
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
  num_lines = 0;
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
  } else {
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
    
    //read_res = read(fd, buffer, sizeof(buffer));
    while (num_lines > 0) {
      // set offset w/ lseek() & read input into buffer
      lseek(fd, offset, SEEK_CUR);
      read_res = read(fd, buffer, sizeof(buffer));
      offset = (size_t) 0; // reset offset
      
      // returned value of 0 indicates an EOF--break from while loop
      if (read_res == ((ssize_t) 0)) {
	break;
      }      
      
      // returned value is negative, meaning there's an error--end the program
      if (read_res < ((size_t) 0)) {
	// display error
	fprintf(stderr, "Error reading: %s\n", strerror(errno));
	close(fd);
	return 1;
      }

      // no errors--we can set read_bytes value & continue w/ program
      read_bytes = (size_t) read_res;

      // resize readbytes for memory allocation when a newline character is detected
      for (i = (size_t) 0; i < read_bytes; i++) {
	c = buffer[i];
	if (c != '\n') {
	  
	} else {
	  
	}
      }

      // allocate memory current_line
      ptr = (char *) malloc(read_bytes);
      if (ptr == NULL) {
	return 1;
      }
      c_line = ptr; // have c_line point to place in memory (in heap)     

      // write characters to c_line pointer/array
      for (i = (size_t) 0; i < read_bytes; i++) {
	c = buffer[i];
	c_line[i] = c;
      }
      offset = read_bytes;

      // allocate memory for lines array
      if (lines_len + ((size_t) 1) > lines_size) {
	if (lines_len == ((size_t) 0) || lines == NULL) {
	  // first allocation
	  lptr = (char **) malloc(FIRST_ARRAY_SIZE * sizeof(char *));
	  if (lptr == NULL) {
	    // todo: print error & deallocate memory
	    return 1;
	  }
	  lines = lptr;
	  lines_size = FIRST_ARRAY_SIZE;
	  solptr = (char *) malloc(FIRST_ARRAY_SIZE * sizeof(size_t));
	  if (solptr == NULL) {
	    // todo: print error & deallocate memory
	    return 1;
	  }
	  size_of_lines = solptr;
	} else {
	  // reallocation
	  lptr = (char **) realloc(lines, lines_size * ((size_t) 2) * sizeof(char *));
	  if (lptr == NULL) {
	    // todo: print error & deallocate memory
	    return 1;
	  }
	  lines = lptr;
	  solptr = (char *) realloc(size_of_lines, lines_size * ((size_t) 2) * sizeof(size_t));
	  if (solptr == NULL) {
	    // todo: print error & deallocate memory
	    return 1;
	  }
	  lines_size *= (size_t) 2;
	  size_of_lines = solptr;	  
	}
      }
      lines[lines_len] = c_line;
      size_of_lines[lines_len] = read_bytes;
      lines_len++;
      c_line = NULL;
      
      num_lines--;
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
    for (i = (size_t) 0; i < lines_len; i++) {
      free(lines[i]);
    }
    free(lines);
    free(size_of_lines);
  }
  // signal success
  return 0;
}
