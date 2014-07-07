#include <comservers.h>
#include <nameserver.h>
#include <interface.h>
#include <clockserver.h>
#include <train.h>
#include <sensors.h>
#include <commandcenter.h>
#include <kernel.h>
#include <bwio.h>
#include <util.h>
#include <debug.h>
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

void outputPutStrClear ( char* str, int *row, int *col, char* buffer, int* index ) {
    int i = 0;
    saveCursorPosition(buffer, index);
    setCursor( *row, *col, buffer, index );
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

void setAllTrainSpeedToOne() {
    char commandstr[3];
    int i = 45;
    for (i = 45; i < 51; i++) {
        commandstr[0] = 1;
        commandstr[1] = (char)i;
        commandstr[2] = 0;
        putstr(COM1, commandstr);
    }
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

void drawTrack ( int *row, int *col ) {
    char buffer[500];
    int index = 0;
    cursorCommand ( "[33m" , buffer, &index ); // set color to yellow
    outputPutStr ( " ................o.....s..s...o....s...o.........o.", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( "                    .       .         .            ", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( "       o.....o....s..o......o..s.o...   .s..o....o.", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( "     .                                .    .       ", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( "    s...o.....o..s...o......o...s...o...s    s.o.o.", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( "  .                o         o            .   .    ", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( " .                   o  .  o               o   o   ", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( ".                      . .                   .  .  ", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( ".                      s.s                    .  . ", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( ".                      ...                    .  . ", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( ".                      s.s                    .  . ", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( ".                     . . .                  .  .  ", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( " .                   o     o               o   o   ", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( "  .                o         o           .   .     ", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( "   s...o......o..s..o.......o..s....o..s   s..o....", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( "     .                              .    .         ", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( "       o.......o...............o..s....s...o.......", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    cursorCommand ( "[37m" , buffer, &index );// set color to white
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
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
    else if ( cmdstr[0] == 'a' ){
        *command = CMD_ASSERT;
        return;
    }
    else if ( strcmp (cmdstr, "pd") == 0 && *argc == 2 ){
        *command = CMD_PREDICT_SENSOR;
        return;
    }
    else if ( strcmp (cmdstr, "fd") == 0 && *argc == 2 ){
        *command = CMD_FIND_DISTANCE;
        return;
    }
    else if ( strcmp (cmdstr, "pf") == 0 && *argc == 2 ){
        *command = CMD_PATH_FIND;
        return;
    }
    else if ( strcmp (cmdstr, "st") == 0 && *argc == 3 ){
        *command = CMD_INIT_TRAIN;
        return;
    }
    else if ( strcmp (cmdstr, "dt") == 0 && *argc == 3 ){
        *command = CMD_TRAIN_DEST;
        return;
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

    server_tid = Create(2, (&PutServer));
    msg_struct.iValue = COM2;
    Send (server_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    server_tid = Create(2, (&GetServer));
    msg_struct.iValue = COM2;
    Send (server_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    server_tid = Create(2, (&PutServer));
    msg_struct.iValue = COM1;
    Send (server_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, msglen);

    server_tid = Create(2, (&GetServer));
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
    row = NEXT_POSITION_X; col = 1;
    outputPutStr ( "NEXT SENSOR:", &row, &col , buffer, &index );
    row = PREV_POSITION_X; col = 1;
    outputPutStr ( "PREV SENSOR:", &row, &col , buffer, &index );
    row = EXPECTED_POSITION_X; col = 1;
    outputPutStr ( "EXPECTED ARRIVAL:", &row, &col , buffer, &index );
    row = ACTUAL_POSITION_X; col = 1;
    outputPutStr ( "ACTUAL ARRVAL:", &row, &col , buffer, &index );

    buffer[(index)++] = 0;
    putstr(COM2, buffer);

    row = TRACK_POSITION_X; col = TRACK_POSITION_Y;
    drawTrack ( &row, &col );

    // Create(2, (&clockServer));
    Create(5, (&clockDisplayTask));
    Create(5, (&IdleDisplayTask));
    Create(5, (&handleCommandTask));

}

void formClockStr (int time_tick, char* buffer) {
    time_tick /= 10;
    int clockTenth = time_tick % 10;
    time_tick /= 10;
    int clockSecond = time_tick % 60;
    time_tick /= 60;
    int clockMinute = time_tick;

    int sechi = ( clockSecond / 10 );
    int seclo = ( clockSecond % 10 );

    buffer[0] = '0' + clockMinute;
    buffer[1] = ':';
    buffer[2] = '0' + sechi;
    buffer[3] = '0' + seclo;
    buffer[4] = '.';
    buffer[5] = '0' + clockTenth ;
    buffer[6] = 0;

}

void clockDisplayTask() {
    char buffer[128] = {0};
    int index = 0;

    char clockstr[10] = {0};
    int clockMinute = 0, clockTenth = 0, clockSecond = 0, sechi, seclo;
    int currentTime = Time()+10;
    FOREVER {
        index = 0;

        int row = CLOCK_POSITION_X, col = CLOCK_POSITION_Y;
        DelayUntil(currentTime);
        clockTenth++;
        if ( clockTenth > 9 ) {
            clockTenth = 0;
            clockSecond++;
        }
        if ( clockSecond > 59 ) {
            clockSecond = 0;
            clockMinute++;
        }
        formClockStr(currentTime, clockstr);
        outputPutStrClear ( clockstr, &row, &col , buffer, &index );

        buffer[(index)++] = 0;
        putstr(COM2, buffer);

        currentTime = currentTime + 10;
    }
}

void IdleDisplayTask() {
    char buffer[128] = {0};
    int index = 0;

    int currentTime = Time()+10;
    FOREVER {
        index = 0;

        Delay(100);
        saveCursorPosition(buffer, &index);
        setCursor( IDLE_POSITION_X, IDLE_POSITION_Y, buffer, &index );
        flushLine (buffer, &index);

        int usage = IdleUsage();

        bwi2a(usage, buffer+index);
        index+=2;
        restoreCursorPosition(buffer, &index);

        buffer[(index)++] = 0;
        putstr(COM2, buffer);

    }
}

void SensorsDisplayTask() {
    // FOREVER{}
    char buffer[512] = {0};
    int index = 0;

    char c;
    int i,j, row, col;
    char prev_sensors_bytes[10] = {0};
    char sensors_bytes[10] = {0};
    int sensorDisplayPosition = 0;

    // int k;
    // unsigned int times[80];
    // for ( k = 0; k < 80; k++) {
    //     times[k] = 0;
    // }
    // int time_index;
    // int cur_time;
    unsigned char sensorStr[5];

    char commandstr[2];
    commandstr[0] = 0;
    commandstr[1] = 50;

    // volatile unsigned int * timer_4_low;
    // timer_4_low = (unsigned int *) ( TIMER4_VALUE_LO );

    FOREVER {
        Delay(10);
        memcpy(prev_sensors_bytes, sensors_bytes, 10);
        getLatestSensors(sensors_bytes);
        for (i = 0; i<10 ; i ++){
            c = sensors_bytes[i];
            if (c == prev_sensors_bytes[i]) {
                continue;
            }
            sensors_bytes[i] = c;
            for (j = 0; j< 8 ; j++) {

                row = SENSORS_POSITION_X; col = sensorDisplayPosition * 4 + 1;
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

                        // time_index = (i / 2) * 16 + sensorNum - 1;
                        // cur_time = Time();
                        // times[time_index] = cur_time;
                        // if ((sensorNum == 13 || sensorNum == 14)&& ('A' + (i / 2)) == 'C') {
                        //     putstr_len(COM1, commandstr, 2 );
                        //     commandstr[1] = 49;
                        //     putstr_len(COM1, commandstr, 2 );
                        //     commandstr[1] = 50;
                        //     bwprintf(COM2, "%d \n", Time());
                        // }
                        // if (sensorNum == 3 && ('A' + (i / 2)) == 'D') {
                        //     setSwitch ( SW_CURVE, 0x8);
                        // }
                        // if ((sensorNum == 1 || sensorNum == 2) &&('A' + (i / 2)) == 'D') {
                        //     setSwitch ( SW_STRAIGHT, 17);
                        //     setSwitch ( SW_CURVE, 13);
                        // }
                        // if (sensorNum == 9 && ('A' + (i / 2)) == 'B') {
                        //     bwprintf(COM2, "%c[2J", 0x1B);
                        //     bwprintf(COM2, "%d \n", Time());
                        //     for (k = 0; k < 80; k++) {
                        //         bwprintf(COM2, "%c | %d | %u\n", 'A' + k/16, 1 + k%16, times[k]);
                        //     }
                        //     Assert();
                        // }
                    }
                    if ( !(i % 2) ) {
                        sensorNum = 8 - j;
                        bwi2a ( sensorNum, sensorStr );
                        outputPutStr ( "0", &row, &col, buffer, &index  );
                        outputPutStr ( sensorStr, &row, &col, buffer, &index  );

                        // time_index = (i / 2) * 16 + sensorNum - 1;
                        // cur_time = Time();
                        // times[time_index] = cur_time;
                        // if ((sensorNum == 13 || sensorNum == 14)&& ('A' + (i / 2)) == 'C') {
                        //     putstr_len(COM1, commandstr, 2 );
                        //     commandstr[1] = 49;
                        //     putstr_len(COM1, commandstr, 2 );
                        //     commandstr[1] = 50;
                        //     bwprintf(COM2, "%d \n", Time());
                        // }
                        // if (sensorNum == 3 && ('A' + (i / 2)) == 'D') {
                        //     setSwitch ( SW_CURVE, 0x8);
                        // }
                        // if ((sensorNum == 1 || sensorNum == 2) &&('A' + (i / 2)) == 'D') {
                        //     setSwitch ( SW_STRAIGHT, 17);
                        //     setSwitch ( SW_CURVE, 13);
                        // }
                        // if (sensorNum == 9 && ('A' + (i / 2)) == 'B') {
                        //     bwprintf(COM2, "%c[2J", 0x1B);
                        //     bwprintf(COM2, "%d \n", Time());
                        //     for (k = 0; k < 80; k++) {
                        //         bwprintf(COM2, "%c | %d | %u\n", 'A' + k/16, 1 + k%16, times[k]);
                        //     }
                        //     Assert();
                        // }
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

void LocationDisplayTask() {
    char buffer[200] = {0};
    int index = 0;

    char c;
    int i,j, row, col;
    int train_info[10] = {-1};
    char sensor_str[20] = {0};

    FOREVER {
        waitTrainInfo(49, train_info);
        int index = 0;

        sensor_str[0] = 'A' + (train_info[0] / 16);
        bwi2a( (train_info[0] % 16) + 1, sensor_str + 1);
        row = PREV_POSITION_X; col = 15;
        outputPutStrClear ( sensor_str, &row, &col , buffer, &index );

        sensor_str[0] = 'A' + (train_info[1] / 16);
        bwi2a( (train_info[1] % 16) + 1, sensor_str + 1);
        row = NEXT_POSITION_X; col = 15;
        outputPutStrClear ( sensor_str, &row, &col , buffer, &index );

        row = EXPECTED_POSITION_X; col = 20;
        formClockStr(train_info[2], sensor_str);
        outputPutStrClear ( sensor_str, &row, &col , buffer, &index );

        row = ACTUAL_POSITION_X; col = 20;
        formClockStr(train_info[3], sensor_str);
        outputPutStrClear ( sensor_str, &row, &col , buffer, &index );

        buffer[(index)++] = 0;
        putstr(COM2, buffer);
    }
}

void handleCommandTask() {

    int tempi;

    char buffer[512] = {0};
    int index = 0;

    char commandStr[64];
    int command_str_index=0, row, col;
    char c;

    char msg[10];
    char reply[10];
    int msglen = 10, rpllen = 10, i;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    // for sensor predicting
    char predict_result[10] = {0};
    int cur_sensor;
    int prediction_len, result;
    char tempstr[20] = {0};

    setCursor( CMD_POSITION_X, CMD_POSITION_Y, buffer, &index);
    buffer[index++] = 0;
    putstr(COM2, buffer);

    Create(3, (&TracksTask));
    putc(COM1, 0x60);
    Delay(100);

    // paht finding shits
    int stopping_sensor;            // returning node
    int stoppong_sensor_dist;       // returning distance
    char sensor_route[20] = {0};    // the sensors the train's gonna

    setAllTrainSpeedToOne();
    // set all train speed to 1 before we do anything

    for ( i=1; i <=18 ; i++) {
        setSwitch ( SW_STRAIGHT, i);
    }
    setSwitch ( SW_CURVE, 3);
    setSwitch ( SW_CURVE, 7);
    setSwitch ( SW_CURVE, 14);
    setSwitch ( SW_CURVE, 18);

    setSwitch ( SW_CURVE, 0x99);
    setSwitch ( SW_STRAIGHT, 0x9A);
    setSwitch ( SW_CURVE, 0x9B);
    setSwitch ( SW_STRAIGHT, 0x9C);

    Delay(600);
    Create(3, (&SensorServer));
    Create(5, (&SensorsDisplayTask));

    Create(3, (&CommandCenterServer));
    Create(5, (&LocationDisplayTask));

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

                    setTrainSpeed( atoi(argv[0]), atoi(argv[1]));
                    // uncomment the following to test starting distance
                    // if (msg_struct.iValue == 12) {
                    //     char tempstr[26] = {0};
                    //     bwi2a ( Time(), tempstr );
                    //     row = 18; col = 1;
                    //     outputPutStrLn (tempstr, &row, &col, buffer, &index );
                    // }

                    break;
                case CMD_REVERSE:
                    outputPutStrLn ( "Reversing train ", &row, &col, buffer, &index );
                    outputPutStr ( argv[0], &row, &col, buffer, &index );
                    reverseTrain( atoi(argv[0]));

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
                case CMD_ASSERT:
                    // train_buffer[train_rindex % BUFFER_SIZE] = 0x61;
                    // train_rindex++;
                    Assert();
                    break;
                case CMD_PREDICT_SENSOR:
                    cur_sensor = atoi(argv[0]);
                    prediction_len = atoi(argv[1]);

                    predictSensor( cur_sensor, prediction_len, predict_result );

                    for (tempi = 0; tempi < prediction_len; tempi++) {
                        bwi2a(predict_result[tempi], tempstr);
                        row = 18; col = 1;
                        outputPutStrLn (tempstr, &row, &col, buffer, &index );
                    }
                    break;
                case CMD_FIND_DISTANCE:
                    result = findDistanceBetweenLandmarks(atoi(argv[0]), atoi(argv[1]), 10);

                    bwi2a(result, tempstr);
                    outputPutStrLn (tempstr, &row, &col, buffer, &index );
                    break;
                case CMD_INIT_TRAIN:
                    result = sensorToInt(argv[1][0], atoi(argv[2]));
                    if (result < 0 || result > 79) {
                        outputPutStrLn ("Invalid Sensor", &row, &col, buffer, &index );
                    }
                    else {
                        outputPutStrLn ("Starting train ", &row, &col, buffer, &index );
                        outputPutStr ( argv[0], &row, &col, buffer, &index );
                        outputPutStr ( " at ", &row, &col, buffer, &index );
                        outputPutStr ( argv[1], &row, &col, buffer, &index );
                        outputPutStr ( argv[2], &row, &col, buffer, &index );
                        initTrainLocation(atoi(argv[0]), result );
                    }
                    break;
                case CMD_TRAIN_DEST:
                    result = sensorToInt(argv[1][0], atoi(argv[2]));
                    if (result < 0 || result > 79) {
                        outputPutStrLn ("Invalid Sensor", &row, &col, buffer, &index );
                    }
                    else {
                        outputPutStrLn ("Sending train ", &row, &col, buffer, &index );
                        outputPutStr ( argv[0], &row, &col, buffer, &index );
                        outputPutStr ( " to ", &row, &col, buffer, &index );
                        outputPutStr ( argv[1], &row, &col, buffer, &index );
                        outputPutStr ( argv[2], &row, &col, buffer, &index );
                        setTrainDestination(atoi(argv[0]), result );
                    }
                    break;
                case CMD_PATH_FIND:
                     stopping_sensor = -1;
                     stoppong_sensor_dist = -1;
                    result = pathFind(
                        atoi(argv[0]),          // current node
                        atoi(argv[1]),          // where it wants to go
                        730,                     // stoping distance
                        &stopping_sensor,       // returning node
                        &stoppong_sensor_dist,  // returning distance
                        sensor_route           // the sensors the train's gonna pass
                    );

                    // if (result >= 0) {
                        bwi2a(stopping_sensor, tempstr);
                        row = 18; col = 1;
                        outputPutStrLn (tempstr, &row, &col, buffer, &index );
                        bwi2a(stoppong_sensor_dist, tempstr);
                        row = 18; col = 1;
                        outputPutStrLn (tempstr, &row, &col, buffer, &index );
                    // }
                    // else {
                    //     bwi2a(123456, tempstr);
                    //     row = 18; col = 1;
                    //     outputPutStrLn (tempstr, &row, &col, buffer, &index );
                    // }
                    // for (tempi = 0; tempi < result; tempi++) {
                    //     bwi2a(sensor_route[tempi], tempstr);
                    //     row = 18; col = 1;
                    //     outputPutStrLn (tempstr, &row, &col, buffer, &index );
                    // }
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
    	}
    	else {
    		putc(COM2, c);
    		commandStr[command_str_index++] = c;
    	}
    }
}
