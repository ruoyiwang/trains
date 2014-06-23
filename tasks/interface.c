#include <comservers.h>
#include <nameserver.h>
#include <interface.h>
#include <clockserver.h>
#include <train.h>
#include <kernel.h>
#include <bwio.h>
#include <util.h>
#include <ts7200.h>

void cursorCommand( char * cmd ) {
    char buffer[10];
    buffer[0] = 0x1B;
    int i = 1;

    while ( *cmd ) {
        buffer[i++] = *(cmd++);
    }
    buffer[i++] = 0;
    putstr( COM2, buffer );
}

void flushScreen () {
    cursorCommand ("[2J");
}

void flushLine () {
    cursorCommand ("[K");
}

void setCursor ( int row, int col ) {
	char buffer[15] = {0};
	int index = 0;

    buffer[index++] = '[';
    if ( row > 9) {
        buffer[index++] = 0x30 + row / 10;
    }
    buffer[index++] = 0x30 + row % 10;
    buffer[index++] = ';';
    if ( col > 9) {
        buffer[index++] = 0x30 + col / 10;
    }
    buffer[index++] = 0x30 + col % 10;
    buffer[index++] = 'H';
    buffer[index++] = 0;
    cursorCommand (buffer);
}

void saveCursorPosition () {
    cursorCommand ("7");
}

void restoreCursorPosition () {
    cursorCommand ("8");
}

void outputPutStrLn ( char* str, int *row, int *col ) {
    int i = 0;
    saveCursorPosition();
    setCursor( *row, *col );
    cursorCommand ("D");
    flushLine ();
    while ( str[i] != '\0' ) {
    	i++;
        (*col)++;
    }
    putstr(COM2, str);
    restoreCursorPosition();
}

void outputPutStr ( char* str, int *row, int *col ) {
    int i = 0;
    saveCursorPosition();
    setCursor(*row, *col );
    while ( str[i] != '\0' ) {
    	i++;
        (*col)++;
    }
    putstr(COM2, str);
    restoreCursorPosition();
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
    char msg[2];
    char reply[2];
    int server_tid, msglen = 2, track_task_id;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = SET_SWITCH;
    reply_struct.value = reply;

    track_task_id = WhoIs(TRACK_TASK);
    msg_struct.iValue = address;
    int r = 0, c = 0;
    if ( getSwCursor ( address, &r, &c ) ) {
        if (state == SW_STRAIGHT) {
            outputPutStr ( "S", &r, &c );
            msg_struct.value[0] = 's';
            Send (track_task_id, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
        }
        else if (state == SW_CURVE) {
            outputPutStr ( "C", &r, &c );
            msg_struct.value[0] = 'c';
            Send (track_task_id, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);
        }
    }
}

void drawSwitchesTable ( int *row, int *col ) {
    cursorCommand ( "[33m" ); // set color to yellow
    outputPutStr ( "| 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 |", row, col );
    (*row)++; *col = SW_POSITION_Y;
    cursorCommand ( "[37m" ); // set color to white
    outputPutStr ( "|    |    |    |    |    |    |    |    |    |    |    |", row, col );
    (*row)++; *col = SW_POSITION_Y;
    cursorCommand ( "[33m" ); // set color to yellow
    outputPutStr ( "| 12 | 13 | 14 | 15 | 16 | 17 | 18 | 99 | 9A | 9B | 9C |", row, col );
    (*row)++; *col = SW_POSITION_Y;
    cursorCommand ( "[37m" ); // set color to white
    outputPutStr ( "|    |    |    |    |    |    |    |    |    |    |    |", row, col );
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


    flushScreen();
    int row = 12, col = 1, i=0;
    outputPutStr ( "OUTPUT: ", &row, &col );
    cursorCommand( "[13;18r" );			//scroll section

    row = SW_POSITION_X-1; col = 1;
    outputPutStr ( "SWITCHES: ", &row, &col );
    row = SENSORS_POSITION_X-1; col = 1;
    outputPutStr ( "SENSORS: ", &row, &col );
    row = SW_POSITION_X; col = SW_POSITION_Y;
    drawSwitchesTable ( &row, &col );
    row = CMD_POSITION_X; col = 1;
    outputPutStr ( "cmd>", &row, &col );

    Create(3, CODE_OFFSET + (&clockDisplayTask));
    Create(5, CODE_OFFSET + (&handleCommandTask));

}

void clockDisplayTask() {
    Create(2, CODE_OFFSET + (&clockServer));
    char clockstr[10];
    int clockMinute = 0, clockTenth = 0, clockSecond = 0, sechi, seclo;
    int currentTime = Time()+10;
    FOREVER {
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
        saveCursorPosition();
        setCursor( CLOCK_POSITION_X, CLOCK_POSITION_Y );
        flushLine ();
        sechi = ( clockSecond / 10 );
        seclo = ( clockSecond % 10 );

        clockstr[0] = '0' + clockMinute;
        clockstr[1] = ':';
        clockstr[2] = '0' + sechi;
        clockstr[3] = '0' + seclo;
        clockstr[4] = '.';
        clockstr[5] = '0' + clockTenth ;
        clockstr[6] = 0;

        putstr(COM2, clockstr);

        restoreCursorPosition();
    }
}

void SensorsTask() {
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
                    sensorStr[0] = 'A' + (i / 2);
                    sensorStr[1] = 0;
                    outputPutStr ( sensorStr, &row, &col );
                    if ( (i % 2) ) {
                        sensorNum = 16 - j;
                        if (sensorNum < 10)
                            outputPutStr ( "0", &row, &col );
                        bwi2a ( sensorNum, sensorStr );
                        outputPutStr ( sensorStr, &row, &col );
                    }
                    if ( !(i % 2) ) {
                        sensorNum = 8 - j;
                        bwi2a ( sensorNum, sensorStr );
                        outputPutStr ( "0", &row, &col );
                        outputPutStr ( sensorStr, &row, &col );
                    }
                    outputPutStr ( " ", &row, &col );
                    sensorDisplayPosition = (sensorDisplayPosition + 1) % SENSORS_DISPLAY_WIDTH;
                }

            }

        }
    }
}

void handleCommandTask() {
    char commandStr[64];
    int index=0, row, col;
    char c;

    char msg[15];
    char reply[15];
    int server_tid, msglen = 2, train_task_id, i;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    Create(5, CODE_OFFSET + (&TracksTask));

    for ( i=1; i <=18 ; i++) {
        setSwitch ( SW_CURVE, i);
    }
    setSwitch ( SW_CURVE, 0x99);
    setSwitch ( SW_STRAIGHT, 0x9A);
    setSwitch ( SW_CURVE, 0x9B);
    setSwitch ( SW_STRAIGHT, 0x9C);

    Create(3, CODE_OFFSET + (&SensorsTask));
    setCursor( CMD_POSITION_X, CMD_POSITION_Y);

    FOREVER {
    	c = getc(COM2);
    	if (c == '\r') {
    		commandStr[index++] = 0;
    		index = 0;

            int command = -1;
            char argv[10][10];
            int argc;
            parseCommand( commandStr, &argc , argv, &command );
            row = 18; col = 1;
            switch (command) {
                case CMD_TRAIN:
                    outputPutStrLn ("Starting train ", &row, &col );
                    outputPutStr ( argv[0], &row, &col );
                    outputPutStr ( " at ", &row, &col );
                    outputPutStr ( argv[1], &row, &col );

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
                    outputPutStrLn ( "Reversing train ", &row, &col );
                    outputPutStr ( argv[0], &row, &col );

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
                    outputPutStrLn ( "Setting switch ", &row, &col );
                    outputPutStr ( argv[0], &row, &col );
                    if ( argv[1][0] == 's' ) {
                        setSwitch ( SW_STRAIGHT, atoi(argv[0]));
                        outputPutStr ( " to straight", &row, &col );
                    }
                    if ( argv[1][0] == 'c' ) {
                        setSwitch ( SW_CURVE, atoi(argv[0]));
                        outputPutStr ( " to curve", &row, &col );
                    }
                    break;
                case CMD_QUIT:
                    // train_buffer[train_rindex % BUFFER_SIZE] = 0x61;
                    // train_rindex++;
                    outputPutStrLn ( "Qutting", &row, &col );
                    break;
                default:
                    outputPutStrLn ( "Invalid input", &row, &col );
                    break;
            }

            setCursor( CMD_POSITION_X, CMD_POSITION_Y);
            flushLine ();
    	}
    	else {
    		putc(COM2, c);
    		commandStr[index++] = c;
    	}
    }
}
