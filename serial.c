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
#define DEVTRYLEN (10)

static int nSerialFd[SERINUM];
static int anOpen[SERINUM] = {SERICLOSE, SERICLOSE};//status of nSerialFds
static char asOpenDevName[SERINUM][DEVNAMELEN];//for serial_stdout_status
static int anOpenDev[DEVTRYLEN];//describes which port is open
static struct termios tioOld[SERINUM];
//static unsigned char *pucRecvbuf;


/***********************************
serial_init()
 initializes constant value
 Argument:void
 Return value:void
************************************/
void serial_init(void) {
  int i;
  for (i=0; i<DEVTRYLEN; i++) {
    anOpenDev[i] = SERICLOSE;
  }
}


/***********************************
serial_open()
 opens one port
 Argument:
   nSeriNum (i): index of nSerialFd
   psDevnam (i): device name ex. "/dev/ttyS0"
   nBaudrate (i): defalut 19200
   nDatabit (i): default 8
   nStopbit (i): defalut 1
   nParity (i): defalut 0
 Return value: status
************************************/
int serial_open(int nSeriNum, char *psDevname, int nBaudrate, int nDatabit, int nStopbit, int nParity)
{
  struct termios tioNew;
  int nBR, nDB, nSB, nPty;
    
  //nSerialFd[nSeriNum] = open(psDevname, O_RDWR | O_NOCTTY);
  nSerialFd[nSeriNum] = open(psDevname, O_RDWR | O_NOCTTY | O_NONBLOCK);
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


/***********************************
serial_try_open_all()
 tries to open all port. max port qty is SERINUM
 sets global value anOpen, asOpenDevName, anOpenDev.
 Argument:
   nBaudrate (i)
   nDatabit (i)
   nStopbit (i)
   nParity (i)
 Return value: status
************************************/
int serial_try_open_all(int nBaudrate, int nDatabit, int nStopbit, int nParity)
{
  int i,j = 0;
  char sStr[DEVNAMELEN];
  char acDevName[DEVTRYLEN][DEVNAMELEN];
  int ret = -1;
  
  for (i=0; i < DEVTRYLEN; i++) {
    sprintf(sStr, "/dev/ttyUSB%d", i);
    strcpy(&acDevName[i][0], sStr);
  }
  
  //printf("try open %s\n", &acDevName[0][i]);
  i=0;
  while (j < SERINUM) {
    //printf("i %d  j%d\n", i, j);
    while( anOpen[j] == SERIOPEN ) {
      j++;
    }
    if( serial_open(j,&acDevName[i][0], nBaudrate, nDatabit, nStopbit, nParity) < 0) {
    } else {
      anOpen[j] = SERIOPEN;
      strncpy(&asOpenDevName[j][0], &acDevName[i][0], DEVNAMELEN);
      anOpenDev[i] = SERIOPEN;
      j++;
      ret = 0;
    }
    do {
      i++;
      if (i >= 10) {
	return ret;
      }
    } while (anOpenDev[i] == SERIOPEN);
    usleep(10000);
    //printf("try open %s\n", &acDevName[i][0]);
  }

  return ret;
}


/***********************************
serial_close()
 close one port
 Argument:
   nSeriNum (i): index of nSerialFd
 Return value:
   status. if already port is open, return -1
 ***********************************/
int serial_close(int nSeriNum)
{
  if ( anOpen[nSeriNum] == SERICLOSE ) {
    return -1;
  }
  tcsetattr(nSerialFd[nSeriNum], TCSANOW, &tioOld[nSeriNum]);
  close(nSerialFd[nSeriNum]);
  anOpen[nSeriNum] = SERICLOSE;
  asOpenDevName[nSeriNum][0] = '\0';
  return 0;
}


/***********************************
serial_close_all()
 close all port
 Argument:void
 Return value:void
 ***********************************/
void serial_close_all(void)
{
  int i;
  for (i=0; i<SERINUM; i++) {
    if (anOpen[i]) {
      serial_close(i);
    }
  }
}


/***********************************
serial_putchar()
 send a char
 Argument:
   nSeriNum (i): index of nSerialFd
   uc (i): a character to send
 Return value:status
 ***********************************/
int serial_putchar(int nSeriNum, unsigned char uc)
{
  if (write(nSerialFd[nSeriNum], &uc, 1) != 1) {
    return -1;
  }
  return 0;
}


/***********************************
serial_putstring()
 send a string
 Argument:
   nSeriNum (i): index of nSerialFd
   pcStr (i): string to send
 Return value:status
 ***********************************/
int serial_putstring(int nSeriNum, char *pcStr)
{
  if (write(nSerialFd[nSeriNum], pcStr, strlen(pcStr)) != 1) {
    return -1;
  }
  return 0;
}


/***********************************
serial_getchar()
 read a char
 Argument:
   nSeriNum (i): index of nSerialFd
   puc (o): received char
 Return value:status
 ***********************************/
int serial_getchar(int nSeriNum, unsigned char *puc)
{
  if (read(nSerialFd[nSeriNum], puc, 1) != 1) {
    return -1;
  }
  return 0;
}

/***********************************
serial_stdout_status()
 printf open/close status
 Argument:void
 Return value:void
***********************************/
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


/***********************************
serial_get_serinum()
 returns max serial port num
 Argument:void
 Return value: SERINUM
***********************************/
int serial_get_serinum(void)
{
  return SERINUM;
}


/***********************************
serial_get_serinum()
 returns if serial port is open
 Argument:
   nSeriNum (i): index of nSerialFd
 Return value: 0/1 --- close/open
***********************************/
int serial_is_open(int nSeriNum)
{
  return anOpen[nSeriNum];
}
