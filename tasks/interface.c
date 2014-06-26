#include <comservers.h>
#include <nameserver.h>
#include <interface.h>
#include <clockserver.h>
#include <train.h>
#include <kernel.h>
#include <bwio.h>
#include <util.h>
#include <ts7200.h>

void cursorCommand( char * cmd, char* buffer, int* index ) {
    *(buffer+((*index)++)) = 0x1B;

    while ( *cmd ) {
        *(buffer+((*index)++)) = *(cmd++);
    }
}

void flushScreen (char* buffer, int* index ) {
    cursorCommand ("[2J", buffer, index);
}

void flushLine (char* buffer, int* index ) {
    cursorCommand ("[K", buffer, index);
}

void setCursor ( int row, int col, char* buffer, int* index ) {
    *(buffer+((*index)++)) = 0x1B;
    *(buffer+((*index)++)) = '[';
    if ( row > 9) {
        *(buffer+((*index)++)) = 0x30 + row / 10;
    }
    *(buffer+((*index)++)) = 0x30 + row % 10;
    *(buffer+((*index)++)) = ';';
    if ( col > 9) {
        *(buffer+((*index)++)) = 0x30 + col / 10;
    }
    *(buffer+((*index)++)) = 0x30 + col % 10;
    *(buffer+((*index)++)) = 'H';

}

void saveCursorPosition (char* buffer, int* index ) {
    cursorCommand ("7", buffer, index);
}

void restoreCursorPosition (char* buffer, int* index ) {
    cursorCommand ("8", buffer, index);
}

void outputPutStrLn ( char* str, int *row, int *col, char* buffer, int* index ) {
    int i = 0;
    saveCursorPosition(buffer, index);
    setCursor( *row, *col, buffer, index );
    cursorCommand ("D", buffer, index);
    flushLine (buffer, index);
    while ( str[i] != '\0' ) {
        *(buffer+((*index)++)) = str[i++];
        (*col)++;
    }
    restoreCursorPosition(buffer, index);
}

void outputPutStr ( char* str, int *row, int *col, char* buffer, int* index ) {
    int i = 0;
    saveCursorPosition(buffer, index);
    setCursor( *row, *col, buffer, index );
    while ( str[i] != '\0' ) {
        *(buffer+((*index)++)) = str[i++];
        (*col)++;
    }
    restoreCursorPosition(buffer, index);
}

int getSwCursor ( int sw, int *row, int *col ) {
    if( sw > 18 ) {
        sw -= 134;
    }
    if ( sw < 1 || sw > 22 ) {
        return false;
    }
    if ( sw < 12 ) {
        *row = SW_POSITION_X + 1;
        *col = 5 * sw -1;
    }
    else {
        *row = SW_POSITION_X + 3;
        *col = 5 * (sw - 11) - 1;
    }
    return true;
}

void setSwitch ( int state, int address ) {
    char msg[10];
    char reply[10];

    char buffer[256] = {0};
    int index = 0;

    int server_tid, msglen = 10, track_task_id;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = SET_SWITCH;
    reply_struct.value = reply;
    track_task_id = WhoIs(TRACK_TASK);
    msg_struct.iValue = address;
    int r = 0, c = 0;
    if ( getSwCursor ( address, &r, &c ) ) {
        if (state == SW_STRAIGHT) {
            outputPutStr ( "S", &r, &c, buffer, &index );
            msg_struct.value[0] = 's';
            Send (track_task_id, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
        }
        else if (state == SW_CURVE) {
            outputPutStr ( "C", &r, &c, buffer, &index );
            msg_struct.value[0] = 'c';
            Send (track_task_id, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
        }
    }
    buffer[(index)++] = 0;
    putstr(COM2, buffer);
}

void drawSwitchesTable ( int *row, int *col , char* buffer, int* index ) {

    cursorCommand ( "[33m" , buffer, index ); // set color to yellow
    outputPutStr ( "| 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 |", row, col, buffer, index );
    (*row)++; *col = SW_POSITION_Y;
    cursorCommand ( "[37m" , buffer, index ); // set color to white
    outputPutStr ( "|    |    |    |    |    |    |    |    |    |    |    |", row, col, buffer, index );
    (*row)++; *col = SW_POSITION_Y;
    cursorCommand ( "[33m" , buffer, index ); // set color to yellow
    outputPutStr ( "| 12 | 13 | 14 | 15 | 16 | 17 | 18 | 99 | 9A | 9B | 9C |", row, col, buffer, index );
    (*row)++; *col = SW_POSITION_Y;
    cursorCommand ( "[37m" , buffer, index );// set color to white
    outputPutStr ( "|    |    |    |    |    |    |    |    |    |    |    |", row, col, buffer, index );
}

unsigned int atoi ( unsigned char *str ) {
    unsigned int value = 0, i = 0;
    while ( str[i] != '\0' ) {
        value *= 10;
        value += (unsigned int) ( str[i] - (unsigned char) '0' );
        i++;
    }
    return value;
}

void parseCommand (char* str, int *argc, char argv[10][10], int* command) {
    int i = 0, j = 0;
    *argc = 0;
    char cmdstr[10];
    while ( str[i] != '\0' && str[i] != '\t' && str[i] != ' ' ) {
        cmdstr[i] = str[i];
        i++;
    }

    while ( 1 ) {
        while ( str[i] == '\t' || str[i] == ' ' ) {
            i++;
        }
        if ( str[i] == '\0' ) {
            break;
        }
        j = 0;
        while ( str[i] != '\0' && str[i] != '\t' && str[i] != ' ' ) {
            argv[*argc][j] = str[i];
            j++;
            i++;
        }
        argv[*argc][j] = '\0';
        (*argc) ++;
    }
	cmdstr[2] = 0;
    if ( strcmp (cmdstr, "tr") == 0 && *argc == 2 ){
        *command = CMD_TRAIN;
    }
    else if ( strcmp (cmdstr, "rv") == 0 && *argc == 1 ){
        *command = CMD_REVERSE;
    }
    else if ( strcmp (cmdstr, "sw") == 0 && *argc == 2 ){
        *command = CMD_SWITCH;
    }
    else if ( cmdstr[0] == 'q' ){
        *command = CMD_QUIT;
        return;
    }
    else {
        *command = CMD_INVALID;
    }

    return;
}

void initInterface() {
    char c;
    char msg[2] = {0};
    char reply[2] = {0};
    int server_tid, msglen = 2;

    char buffer[700] = {0};
    int index = 0;

    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    server_tid = Create(2, CODE_OFFSET + (&PutServer));
    msg_struct.iValue = COM2;
    Send (server_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    server_tid = Create(2, CODE_OFFSET + (&GetServer));
    msg_struct.iValue = COM2;
    Send (server_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    server_tid = Create(2, CODE_OFFSET + (&PutServer));
    msg_struct.iValue = COM1;
    Send (server_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    server_tid = Create(2, CODE_OFFSET + (&GetServer));
	msg_struct.iValue = COM1;
    Send (server_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    // bwprintf(COM2, "WEREWREWREW\n\n");
    flushScreen(buffer, &index);
    int row = 12, col = 1, i=0;
    outputPutStr ( "OUTPUT: ", &row, &col , buffer, &index );
    cursorCommand( "[13;18r" , buffer, &index );			//scroll section

    row = SW_POSITION_X-1; col = 1;
    outputPutStr ( "SWITCHES: ", &row, &col , buffer, &index );
    row = SENSORS_POSITION_X-1; col = 1;
    outputPutStr ( "SENSORS: ", &row, &col , buffer, &index );
    row = SW_POSITION_X; col = SW_POSITION_Y;
    drawSwitchesTable ( &row, &col, buffer, &index );
    row = CMD_POSITION_X; col = 1;
    outputPutStr ( "cmd>", &row, &col , buffer, &index );

    buffer[(index)++] = 0;
    putstr(COM2, buffer);

    // Create(2, CODE_OFFSET + (&clockServer));
    Create(3, CODE_OFFSET + (&clockDisplayTask));
    Create(5, CODE_OFFSET + (&handleCommandTask));

}

void clockDisplayTask() {
    Create(2, CODE_OFFSET + (&clockServer));

    char buffer[128] = {0};
    int index = 0;

    char clockstr[10];
    int clockMinute = 0, clockTenth = 0, clockSecond = 0, sechi, seclo;
    int currentTime = Time()+10;
    FOREVER {
        index = 0;

        int row = 18, col = 1;
        DelayUntil(currentTime);
        currentTime = currentTime + 10;
        clockTenth++;
        if ( clockTenth > 9 ) {
            clockTenth = 0;
            clockSecond++;
        }
        if ( clockSecond > 59 ) {
            clockSecond = 0;
            clockMinute++;
        }
        saveCursorPosition(buffer, &index);
        setCursor( CLOCK_POSITION_X, CLOCK_POSITION_Y, buffer, &index );
        flushLine (buffer, &index);
        sechi = ( clockSecond / 10 );
        seclo = ( clockSecond % 10 );

        buffer[(index)++] = '0' + clockMinute;
        buffer[(index)++] = ':';
        buffer[(index)++] = '0' + sechi;
        buffer[(index)++] = '0' + seclo;
        buffer[(index)++] = '.';
        buffer[(index)++] = '0' + clockTenth ;

        restoreCursorPosition(buffer, &index);

        buffer[(index)++] = 0;
        putstr(COM2, buffer);

    }
}

void SensorsTask() {
    // FOREVER{}
    char buffer[512] = {0};
    int index = 0;

    char clockstr[10];
    char c;
    int i,j, row, col;
    // char sensors_bytes[10];
    int sensorDisplayPosition = 0;
    FOREVER {
        Delay(30);
        putc(COM1, 133);
        for (i = 0; i<10 ; i ++){
            c = getc(COM1);
            for (j = 0; j< 8 ; j++) {

                row = SENSORS_POSITION_X; col = sensorDisplayPosition * 4 + 1;
                unsigned char sensorStr[5];
                int sensorNum;
                if ( c & ( 1 << j ) ){
                    index = 0;
                    sensorStr[0] = 'A' + (i / 2);
                    sensorStr[1] = 0;
                    outputPutStr ( sensorStr, &row, &col, buffer, &index );
                    if ( (i % 2) ) {
                        sensorNum = 16 - j;
                        if (sensorNum < 10)
                            outputPutStr ( "0", &row, &col, buffer, &index  );
                        bwi2a ( sensorNum, sensorStr );
                        outputPutStr ( sensorStr, &row, &col, buffer, &index  );
                    }
                    if ( !(i % 2) ) {
                        sensorNum = 8 - j;
                        bwi2a ( sensorNum, sensorStr );
                        outputPutStr ( "0", &row, &col, buffer, &index  );
                        outputPutStr ( sensorStr, &row, &col, buffer, &index  );
                    }
                    outputPutStr ( " ", &row, &col, buffer, &index  );
                    sensorDisplayPosition = (sensorDisplayPosition + 1) % SENSORS_DISPLAY_WIDTH;

                    buffer[(index)++] = 0;
                    putstr(COM2, buffer);
                }
            }

        }
    }
}

void handleCommandTask() {

    char buffer[512] = {0};
    int index = 0;

    char commandStr[64];
    int command_str_index=0, row, col;
    char c;

    char msg[15];
    char reply[15];
    int server_tid, msglen = 2, train_task_id, i;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;


    setCursor( CMD_POSITION_X, CMD_POSITION_Y, buffer, &index);
    buffer[index++] = 0;
    putstr(COM2, buffer);

    Create(3, CODE_OFFSET + (&TracksTask));
    for ( i=1; i <=18 ; i++) {
        setSwitch ( SW_CURVE, i);
    }
    setSwitch ( SW_CURVE, 0x99);
    setSwitch ( SW_STRAIGHT, 0x9A);
    setSwitch ( SW_CURVE, 0x9B);
    setSwitch ( SW_STRAIGHT, 0x9C);

    Create(3, CODE_OFFSET + (&SensorsTask));

    FOREVER {
    	c = getc(COM2);
    	if (c == '\r') {
    		commandStr[command_str_index++] = 0;
    		command_str_index = 0;

            int command = -1;
            char argv[10][10];
            int argc;
            parseCommand( commandStr, &argc , argv, &command );
            row = 18; col = 1;
            index = 0;

            switch (command) {
                case CMD_TRAIN:
                    outputPutStrLn ("Starting train ", &row, &col, buffer, &index );
                    outputPutStr ( argv[0], &row, &col, buffer, &index );
                    outputPutStr ( " at ", &row, &col, buffer, &index );
                    outputPutStr ( argv[1], &row, &col, buffer, &index );

                    genTrainName(atoi(argv[0]), msg);
                    train_task_id = WhoIs(msg);
                    if (train_task_id == -1) {
                        train_task_id = Create(4, CODE_OFFSET + (&TrainTask));
                        msg_struct.iValue = atoi(argv[0]);
                        Send (train_task_id, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
                    }
                    msg_struct.type = TRAIN_SET_SPEED;
                    msg_struct.iValue = atoi(argv[1]);
                    Send (train_task_id, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

                    break;
                case CMD_REVERSE:
                    outputPutStrLn ( "Reversing train ", &row, &col, buffer, &index );
                    outputPutStr ( argv[0], &row, &col, buffer, &index );

                    genTrainName(atoi(argv[0]), msg);
                    train_task_id = WhoIs(msg);
                    if (train_task_id == -1) {
                        train_task_id = Create(4, CODE_OFFSET + (&TrainTask));
                        msg_struct.iValue = atoi(argv[0]);
                        Send (train_task_id, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
                    }
                    msg_struct.type = TRAIN_REVERSE;
                    Send (train_task_id, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

                    break;
                case CMD_SWITCH:
                    outputPutStrLn ( "Setting switch ", &row, &col, buffer, &index );
                    outputPutStr ( argv[0], &row, &col, buffer, &index );
                    if ( argv[1][0] == 's' ) {
                        setSwitch ( SW_STRAIGHT, atoi(argv[0]));
                        outputPutStr ( " to straight", &row, &col, buffer, &index );
                    }
                    if ( argv[1][0] == 'c' ) {
                        setSwitch ( SW_CURVE, atoi(argv[0]));
                        outputPutStr ( " to curve", &row, &col, buffer, &index );
                    }
                    break;
                case CMD_QUIT:
                    // train_buffer[train_rindex % BUFFER_SIZE] = 0x61;
                    // train_rindex++;
                    outputPutStrLn ( "Qutting", &row, &col, buffer, &index );
                    break;
                default:
                    outputPutStrLn ( "Invalid input", &row, &col, buffer, &index );
                    break;
            }

            setCursor( CMD_POSITION_X, CMD_POSITION_Y, buffer, &index );
            flushLine (buffer, &index);
            buffer[(index)++] = 0;
            putstr(COM2, buffer);
            Assert();
    	}
    	else {
    		putc(COM2, c);
    		commandStr[command_str_index++] = c;
    	}
    }
}
