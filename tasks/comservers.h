#ifndef __COM_SERVER__
#define __COM_SERVER__

#define BUFFER_SIZE 2048


#define COM2_GETC_NOTIFIER  0x30
#define COM2_PUTC_NOTIFIER  0x30
#define COM2_GETC_REQUEST   0x31
#define COM2_PUTC_REQUEST   0x33


#define COM2_GET_SERVER     "COM2 GET SERVER"

void Com2PutServerNotifier();
void Com2PutServer();

void Com2GetServerNotifier();
void Com2GetServer();

void putc(int COM, char c);
char getc(int COM);

#endif
