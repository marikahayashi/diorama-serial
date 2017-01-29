#include <stdio.h>
#include <unistd.h>
#include "sio.h"

int main(void)
{
  unsigned char ucRecv;

  //disable buffering of printf
  setvbuf(stdout, (char *)NULL, _IONBF, 0);
  
  serial_open("/dev/ttyUSB0", 19200, 8, 1, 0);

  while (1) {
    usleep(1000);
    if (serial_getchar(&ucRecv) < 0) {
      perror("serial_getchar");
    }
    serial_putchar(ucRecv);
    printf("%c", ucRecv);
    if (ucRecv == 'q') {
      break;
    }
  }

  serial_close();

  return 0;
}
