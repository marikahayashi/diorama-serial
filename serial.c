#include <stdio.h>
#include <termios.h>
#include <sys/signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>
#include "serial.h"

#define SERINUM (2)
#define SERICLOSE (0)
#define SERIOPEN (1)
#define DEVNAMELEN (13)
static int nSerialFd[SERINUM];
static int anOpen[SERINUM] = {SERICLOSE, SERICLOSE};
static char asOpenDevName[SERINUM][DEVNAMELEN] = {"", ""};
static struct termios tioOld[SERINUM];
//static unsigned char *pucRecvbuf;

int serial_open(int nSeriNum, char *psDevname, int nBaudrate, int nDatabit, int nStopbit, int nParity)
{
  struct termios tioNew;
  int nBR, nDB, nSB, nPty;
    
  nSerialFd[nSeriNum] = open(psDevname, O_RDWR | O_NOCTTY);
  if (nSerialFd[nSeriNum] < 0) {
    //perror("open");
    return -1;
  }

  if (tcgetattr(nSerialFd[nSeriNum], &tioOld[nSeriNum]) < 0) {
    close(nSerialFd[nSeriNum]);
    return -1;
  }

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
  case 57600:
    nBR = B57600;
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

  if (tcflush(nSerialFd[nSeriNum], TCIFLUSH) < 0) {
    close(nSerialFd[nSeriNum]);
    return -1;
  }
  if (tcsetattr(nSerialFd[nSeriNum], TCSANOW, &tioNew) < 0) {
    close(nSerialFd[nSeriNum]);
    return -1;
  }

  anOpen[nSeriNum] = SERIOPEN;
  return 0;
}

int serial_try_open_all(int nBaudrate, int nDatabit, int nStopbit, int nParity)
{
  int i,j = 0;
  char acDevName[10][DEVNAMELEN] = {
  "/dev/ttyUSB0", "/dev/ttyUSB1",
  "/dev/ttyUSB2", "/dev/ttyUSB3",
  "/dev/ttyUSB4", "/dev/ttyUSB5",
  "/dev/ttyUSB6", "/dev/ttyUSB7",
  "/dev/ttyUSB8", "/dev/ttyUSB9"};

  //printf("try open %s\n", &acDevName[0][i]);
  while (j < SERINUM - 1) {
    if( serial_open(j,&acDevName[i][0], nBaudrate, nDatabit, nStopbit, nParity) < 0) {
    } else {
      anOpen[j] = SERIOPEN;
      strncpy(&asOpenDevName[j][0], &acDevName[i][0], DEVNAMELEN);
      j++;
    }
    i++;
    if (i > 10) {
      i = 0;
    }
    usleep(10000);
    //printf("try open %s\n", &acDevName[i][0]);
  }

  return 0;
}

void serial_close(int nSeriNum)
{
  tcsetattr(nSerialFd[nSeriNum], TCSANOW, &tioOld[nSeriNum]);
  close(nSerialFd[nSeriNum]);
  anOpen[nSeriNum] = SERICLOSE;
  asOpenDevName[nSeriNum][0] = '\0';
}

void serial_close_all(void)
{
  int i;
  for (i=0; i<SERINUM; i++) {
    if (anOpen[i]) {
      serial_close(i);
    }
  }
}

int serial_putchar(int nSeriNum, unsigned char uc)
{
  if (write(nSerialFd[nSeriNum], &uc, 1) != 1) {
    return -1;
  }
  return 0;
}

int serial_putstring(int nSeriNum, char *pcStr)
{
  if (write(nSerialFd[nSeriNum], pcStr, strlen(pcStr)) != 1) {
    return -1;
  }
  return 0;
}

int serial_getchar(int nSeriNum, unsigned char *puc)
{
  if (read(nSerialFd[nSeriNum], puc, 1) != 1) {
    return -1;
  }
  return 0;
}

void serial_stdout_status(void)
{
  int i;
  for (i=0; i<SERINUM; i++) {
    if (anOpen[i] == SERIOPEN) {
      printf("%d: OPEN, ", i);
    } else {
      printf("%d: CLOSE, ", i);
    }
    printf("%s  ", asOpenDevName[i]);
  }
  printf("\n");
}

int serial_get_serinum(void)
{
  return SERINUM;
}

int serial_is_open(int nSeriNum)
{
  return anOpen[nSeriNum];
}
