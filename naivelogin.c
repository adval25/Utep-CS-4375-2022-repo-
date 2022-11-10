#include <stdio.h>
#include <unistd.h>

#if !defined(LOGIN_SECRET)
#define LOGIN_SECRET "password"
#endif

static int read_character(char *c) {
  char buf;
  ssize_t res;
  //printf("enter\n");
  res = read(0, &buf, (size_t) 1);
  //printf("after read\n");  
  if (res == ((ssize_t) 1)) {
    *c = buf;
    //printf("here1\n");
    return 0;
  }
  //printf("here2\n");
  return -1;
}

int main(int argc, char **argv) {
  char secret[] = LOGIN_SECRET;
  int i;
  char c;
  
  for (i=0; secret[i]!='\0'; i++) {
    //printf("%d\n",i);
    if (read_character(&c) < 0) {
      //printf("not enough cash stranger\n");
      return 1;
    }
    if (c != secret[i]){
      //printf("%c:char\n",c);
      return 1;
    }
    //printf("%d:\n",i);
  }
  //printf("pass on by, yeah?\n");
  return 0;
}

