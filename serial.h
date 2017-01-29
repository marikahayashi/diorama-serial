int serial_open(char *psDevname, int nBaudrate, int nDatabit, int nStopbit, int nParity);
void serial_close(void);
int serial_putchar(unsigned char uc);
int serial_putstring(char *pcStr);
int serial_getchar(unsigned char *puc);
