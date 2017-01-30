#include <stdio.h>
#include <unistd.h>
#include "serial.h"


int main(void)
{
  unsigned char ucRecv = '\0';
  int i;
  
  //disable buffering of printf
  setvbuf(stdout, (char *)NULL, _IONBF, 0);

  serial_init();
  serial_try_open_all(19200, 8, 1, 0);
  serial_stdout_status();
  
  while (1) {
    usleep(500000);
    
    for (i=0; i < serial_get_serinum(); i++) {
      if (serial_is_open(i)) {
	
	if (serial_getchar(i,&ucRecv) < 0) {
	  perror("serial_getchar");
	  if (serial_close(i) == 0) {
	    //if getchar failed, this program assumes
	    //that device is disconnected.
	    //then close port.
	    serial_stdout_status();
	  }
	}
	if (serial_putchar(i,ucRecv) < 0) {
	  perror("serial_putchar");
	  if (serial_close(i) == 0) {
	    //if putchar failed, this program assumes
	    //that device is disconnected.
	    //then close port.
	    serial_stdout_status();
	  }
	}
	printf("%c", ucRecv);
	if (ucRecv == 'q') {
	  break;
	}
	
      } else { //if there is a closed port, try open
	if( serial_try_open_all(19200, 8, 1, 0) == 0) {
	  serial_stdout_status();
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
