#ifndef __COM_SERVER__
#define __COM_SERVER__

#define BUFFER_SIZE 8192


#define GETC_NOTIFIER  0
#define PUTC_NOTIFIER  1
#define GETC_REQUEST   2
#define PUTC_REQUEST   3
#define PUTSTR_REQUEST 4
#define PUTSTR_LEN_REQUEST 5
#define GETC_TIMEOUT 6
#define GETC_TIMEOUT_REQUEST 7

#define COM2_GET_SERVER     "COM2 GET SERVER"
#define COM2_PUT_SERVER     "COM2 PUT SERVER"

#define COM1_GET_SERVER     "COM1 GET SERVER"
#define COM1_PUT_SERVER     "COM1 PUT SERVER"

void Com1PutServerNotifier();
void Com2PutServerNotifier();
void PutServer();

void Com1GetServerNotifier();
void Com2GetServerNotifier();
void GetServer();

void putc(int COM, char c);
void putstr(int COM, char* str);
void putstr_len(int COM, char* str, int msglen);
char getc(int COM);

#endif
