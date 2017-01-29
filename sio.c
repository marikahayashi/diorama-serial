#include <stdio.h>
#include <termios.h>
#include <sys/signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>

int nSerialFd;
struct termios tioOld;
unsigned char *pucRecvbuf;

int serial_open(char *psDevname, int nBaudrate, int nDatabit, int nStopbit, int nParity)
{
  struct termios tioNew;
  int nBR, nDB, nSB, nPty;
  
  nSerialFd = open(psDevname, O_RDWR | O_NOCTTY);
  if (nSerialFd < 0) {
    perror("open");
    return -1;
  }

  tcgetattr(nSerialFd, &tioOld);

  tioNew.c_iflag = 0;
  tioNew.c_oflag = 0;
  tioNew.c_cflag = 0;
  tioNew.c_lflag = 0;
  tioNew.c_line = 0;
  bzero(tioNew.c_cc, sizeof(tioNew.c_cc));
  switch (nBaudrate) {
  case 9600:
    nBR = B9600;
    break;
  case 19200:
    nBR = B19200;
    break;
  case 38400:
    nBR = B38400;
    break;
  case 115200:
    nBR = B115200;
    break;
  default:
    nBR = B19200;
    break;
  }
  switch (nDatabit) {
  case 7:
    nDB = CS7;
    break;
  case 8:
    nDB = CS8;
    break;
  default:
    nDB = CS8;
    break;
  }
  switch (nStopbit) {
  case 1:
    nSB = 0;
    break;
  case 2:
    nSB = CSTOPB;
    break;
  default:
    nSB = 0;
    break;
  }
  switch (nParity) {
  case 0:
    nPty = 0;
    break;
  case 1:
    nPty = PARENB;
    break;
  case 2:
    nPty = PARENB | PARODD;
    break;
  default:
    nPty = 0;
    break;
  }
  tioNew.c_cflag = CLOCAL | CREAD | nBR | nDB | nSB | nPty;
  tioNew.c_cc[VTIME] = 0;
  tioNew.c_cc[VMIN] = 1;
  
  tcflush(nSerialFd, TCIFLUSH);
  tcsetattr(nSerialFd, TCSANOW, &tioNew);
  
  return 0;
}

void serial_close(void)
{
  tcsetattr(nSerialFd, TCSANOW, &tioOld);
  close(nSerialFd);
}

int serial_putchar(unsigned char uc)
{
  if (write(nSerialFd, &uc, 1) != 1) {
    return -1;
  }
  return 0;
}

int serial_putstring(char *pcStr)
{
  if (write(nSerialFd, pcStr, strlen(pcStr)) != 1) {
    return -1;
  }
  return 0;
}

int serial_getchar(unsigned char *puc)
{
  if (read(nSerialFd, puc, 1) != 1) {
    return -1;
  }
  return 0;
}
