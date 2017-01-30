#include <stdio.h>
#include <unistd.h>
#include "serial.h"

int main(void)
{
  unsigned char ucRecv;
  int i;
  
  //disable buffering of printf
  setvbuf(stdout, (char *)NULL, _IONBF, 0);

  serial_stdout_status();
  
  serial_try_open_all(19200, 8, 1, 0);

  serial_stdout_status();
  
  while (1) {
    usleep(1000);
    for (i=0; i < serial_get_serinum(); i++) {
      if (serial_is_open(i)) {
	if (serial_getchar(i,&ucRecv) < 0) {
	  perror("serial_getchar");
	  serial_close(i);
	}
	if (serial_putchar(0,ucRecv) < 0) {
	  perror("serial_putchar");
	  serial_close(i);
	}
	printf("%c", ucRecv);
	if (ucRecv == 'q') {
	  break;
	}
      }
    }
    if (ucRecv == 'q') {
      break;
    }
  }

  serial_close_all();
  serial_stdout_status();
  
  return 0;
}
