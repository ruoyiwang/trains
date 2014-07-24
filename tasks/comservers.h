#ifndef __COM_SERVER__
#define __COM_SERVER__

#define BUFFER_SIZE 8192


#define GETC_NOTIFIER  0
#define PUTC_NOTIFIER  1
#define GETC_REQUEST   159
#define PUTC_REQUEST   3
#define PUTSTR_REQUEST 4
#define PUTSTR_LEN_REQUEST 5
#define GETC_TIMEOUT 6
#define GETC_TIMEOUT_REQUEST 7

#define COM2_GET_SERVER     "C2G"
#define COM2_PUT_SERVER     "C2P"

#define COM1_GET_SERVER     "C1G"
#define COM1_PUT_SERVER     "C1P"

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
