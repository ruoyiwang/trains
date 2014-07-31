#include <comservers.h>
#include <nameserver.h>
#include <interface.h>
#include <clockserver.h>
#include <track.h>
#include <train.h>
#include <sensors.h>
#include <commandcenter.h>
#include <kernel.h>
#include <bwio.h>
#include <util.h>
#include <debug.h>
#include <ts7200.h>
#include <display.h>
#include <multidestination.h>

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
    if ( row > 99) {
        *(buffer+((*index)++)) = 0x30 + row / 100;
    }
    if ( row > 9) {
        *(buffer+((*index)++)) = 0x30 + row %100 / 10;
    }
    *(buffer+((*index)++)) = 0x30 + row % 10;
    *(buffer+((*index)++)) = ';';
    if ( col > 99) {
        *(buffer+((*index)++)) = 0x30 + col / 100;
    }
    if ( col > 9) {
        *(buffer+((*index)++)) = 0x30 + col %100 / 10;
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

void DebugPutStr ( char* fmt, ... ) {
    va_list va;
    char c;
    char buffer[700] = {0};
    char temp[20] = {0};
    int index = 0;
    char* str;
    int i;
    saveCursorPosition(buffer, &index);
    setCursor( CMD_POSITION_X-1, 1, buffer, &index );
    cursorCommand ("D", buffer, &index);
    flushLine (buffer, &index);
    va_start(va,fmt);
    while ( (c = *(fmt++)) ){
        switch(c){
            case 's':
                str = va_arg( va, char* );
                i = 0;
                while ( str[i] != '\0' ) {
                    *(buffer+(index++)) = str[i++];
                }
                break;
            case 'c':
                c = va_arg( va, char );
                *(buffer+(index++)) = c;
                break;
            case 'd':
                i = va_arg( va, int );
                bwi2a(i, temp);
                i = 0;
                while ( temp[i] != '\0' ) {
                    *(buffer+(index++)) = temp[i++];
                }
                break;
            default:
                bwprintf(COM2, "\n\n\n\n\n\n\nfmlllllllllllllllllllllllll DEBUGPUTSTR %d", c);
                return;
        }
    }
    va_end(va);
    restoreCursorPosition(buffer, &index);
    buffer[(index)++] = 0;
    putstr(COM2, buffer);
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
    for (i = 40; i < 60; i++) {
        commandstr[0] = 16;
        commandstr[1] = (char)i;
        putstr_len(COM1, commandstr, 2);
    }
}

void setSwitch ( int state, int address ) {
    setSwitchForce(state, address, false);
}

void setSwitchForce ( int state, int address, int force ) {
    char msg[10];
    char reply[10];

    char buffer[200] = {0};
    int index = 0;

    int msglen = 2, rpllen = 10, track_task_id;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    msg_struct.type = SET_SWITCH;
    reply_struct.value = reply;
    track_task_id = WhoIs(TRACK_TASK);
    msg_struct.iValue = address;
    int r = 0, c = 0;
    if ( getSwCursor ( address, &r, &c ) ) {
        if (state == SW_STRAIGHT) {
            msg_struct.value[0] = 's';
            msg_struct.value[1] = force;
            Send (track_task_id, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

            outputPutStr ( "S", &r, &c, buffer, &index );
            if (address <= 18){
                get_cordinates(address + 79, &r, &c);
            }
            else{
                get_cordinates(address, &r, &c);
            }
            r+=TRACK_POSITION_X-1;
            c+=TRACK_POSITION_Y-1;
            cursorCommand ( "[33m" , buffer, &index ); // set color to white
            outputPutStr ( "s", &r, &c, buffer, &index );
            cursorCommand ( "[0m" , buffer, &index ); // set color to white
        }
        else if (state == SW_CURVE) {
            msg_struct.value[0] = 'c';
            msg_struct.value[1] = force;
            Send (track_task_id, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

            outputPutStr ( "C", &r, &c, buffer, &index );
            if (address <= 18){
                get_cordinates(address + 79, &r, &c);
            }
            else{
                get_cordinates(address, &r, &c);
            }
            r+=TRACK_POSITION_X-1;
            c+=TRACK_POSITION_Y-1;
            cursorCommand ( "[33m" , buffer, &index ); // set color to white
            outputPutStr ( "c", &r, &c, buffer, &index );
            cursorCommand ( "[0m" , buffer, &index ); // set color to white
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
    cursorCommand ( "[31m" , buffer, &index ); // set color to yellow
    outputPutStr ( " ................o....s...s...o....s...o.........o.", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( "                    .       .         .            ", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
    buffer[index++] = 0; putstr(COM2, buffer); index = 0;
    outputPutStr ( "       o.....o....s..o......o..s.o...   s...o....o.", row, col, buffer, &index ); (*row)++; *col = TRACK_POSITION_Y;
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

unsigned int atoi ( char *str ) {
    unsigned int value = 0, i = 0;
    while ( str[i] != '\0' ) {
        value *= 10;
        value += (unsigned int) ( str[i] - (char) '0' );
        i++;
    }
    return value;
}

void parseCommand (char* str, int *argc, char argv[10][10], int* command) {
    int i = 0, j = 0;
    *argc = 0;
    char cmdstr[10] = {0};
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
	// cmdstr[2] = 0;
    if ( strcmp (cmdstr, "tr") == 0 && *argc == 2 ){
        *command = CMD_TRAIN;
    }
    else if ( strcmp (cmdstr, "rv") == 0 && *argc == 1 ){
        *command = CMD_REVERSE;
    }
    else if ( strcmp (cmdstr, "sw") == 0 && *argc == 2 ){
        *command = CMD_SWITCH;
    }
    else if ( strcmp (cmdstr, "pd") == 0 && *argc == 2 ){
        *command = CMD_PREDICT_SENSOR;
        return;
    }
    else if ( strcmp (cmdstr, "fd") == 0 && *argc == 2 ){
        *command = CMD_FIND_DISTANCE;
        return;
    }
    else if ( strcmp (cmdstr, "di") == 0 && *argc == 3 ){
        *command = CMD_PF_DIJKSTRA;
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
    else if ( strcmp (cmdstr, "dt") == 0 && *argc == 4 ){
        *command = CMD_TRAIN_DEST;
        return;
    }
    else if ( strcmp (cmdstr, "dtm") == 0 && *argc >= 3 ){
        *command = CMD_TRAIN_DEST_MULT;
        return;
    }
    else if ( strcmp (cmdstr, "sm") == 0 && *argc == 2 ){
        *command = CMD_SHORT_MOVE_TIME;
        return;
    }
    else if ( strcmp (cmdstr, "it") == 0 && *argc == 1 ){
        *command = CMD_INIT_TRACK;
        return;
    }
    else if ( strcmp (cmdstr, "cn") == 0 ){
        *command = CMD_CREATE_SENSOR_NOTIFIER;
        return;
    }
    else if ( strcmp (cmdstr, "sd") == 0 ){
        *command = CMD_SET_STOPPING_DIST;
        return;
    }
    else if ( strcmp (cmdstr, "mm") == 0 ){
        *command = CMD_SET_SHORT_MULT;
        return;
    }
    else if ( strcmp (cmdstr, "rn") == 0 ){
        *command = CMD_RESERVE_NODE;
        return;
    }
    else if ( strcmp (cmdstr, "fn") == 0 ){
        *command = CMD_FREE_NODE;
        return;
    }
    else if ( strcmp (cmdstr, "lrn") == 0 ){
        *command = CMD_LIST_RESERVED_NODE;
        return;
    }
    else if ( strcmp (cmdstr, "npnc") == 0 && *argc == 3 ){
        *command = CMD_NEXT_SENSORS_CHECK;
        return;
    }
    else if ( cmdstr[0] == 'A' ){
        *command = CMD_ASSERT;
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
    int row = 24, col = 1;
    outputPutStr ( "OUTPUT: ", &row, &col , buffer, &index );
    cursorCommand( "[25;49r" , buffer, &index );			//scroll section

    row = SW_POSITION_X-1; col = 1;
    outputPutStr ( "SWITCHES: ", &row, &col , buffer, &index );
    row = SENSORS_POSITION_X-1; col = 1;
    outputPutStr ( "SENSORS: ", &row, &col , buffer, &index );
    row = SW_POSITION_X; col = SW_POSITION_Y;
    drawSwitchesTable ( &row, &col, buffer, &index );
    row = CMD_POSITION_X; col = 1;
    outputPutStr ( "cmd>", &row, &col , buffer, &index );
    row = TRAIN_TABLE_X; col = 1;
    cursorCommand ( "[33m" , buffer, &index ); // set color to yellow
    outputPutStr ( "ID | NEXT | PREV | LOCATION | EXPECTED ARRIVAL | ACTUAL ARRIVAL | DEST ", &row, &col , buffer, &index );
    cursorCommand ( "[37m" , buffer, &index ); // set color to yellow

    // outputPutStr ( "NEXT SENSOR:", &row, &col , buffer, &index );
    // row = PREV_POSITION_X; col = 1;
    // outputPutStr ( "PREV SENSOR:", &row, &col , buffer, &index );
    // row = EXPECTED_POSITION_X; col = 1;
    // outputPutStr ( "EXPECTED ARRIVAL:", &row, &col , buffer, &index );
    // row = ACTUAL_POSITION_X; col = 1;
    // outputPutStr ( "ACTUAL ARRVAL:", &row, &col , buffer, &index );

    buffer[(index)++] = 0;
    putstr(COM2, buffer);

    row = TRACK_POSITION_X; col = TRACK_POSITION_Y;
    drawTrack ( &row, &col );

    // Create(2, (&clockServer));
    Create(4, (&clockDisplayTask));
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
    int currentTime = Time()+10;
    FOREVER {
        index = 0;

        int row = CLOCK_POSITION_X, col = CLOCK_POSITION_Y;
        DelayUntil(currentTime);
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

    FOREVER {
        index = 0;

        Delay(100);
        saveCursorPosition(buffer, &index);
        setCursor( IDLE_POSITION_X, IDLE_POSITION_Y, buffer, &index );
        flushLine (buffer, &index);

        int usage = IdleUsage();

        bwi2a(usage, buffer+index);
        if (usage < 10) {
            index++;
        }
        else if (usage > 99){
            index += 3;
        }
        else {
            index+=2;
        }
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
    char sensorStr[10] = {0};

    char commandstr[2];
    commandstr[0] = 0;
    commandstr[1] = 50;

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
                    if ( (i % 2) ) {
                        sensorNum = 16 - j;
                        if (sensorNum < 10){
                            sensorStr[1] = '0';
                            bwi2a ( sensorNum, sensorStr+2 );
                        }
                        else {
                            bwi2a ( sensorNum, sensorStr+1 );
                        }
                        outputPutStr ( sensorStr, &row, &col, buffer, &index  );
                    }
                    if ( !(i % 2) ) {
                        sensorNum = 8 - j;
                        sensorStr[1] = '0';
                        bwi2a ( sensorNum, sensorStr+2 );
                        outputPutStr ( sensorStr, &row, &col, buffer, &index  );
                    }
                    outputPutStr ( "     ", &row, &col, buffer, &index  );
                    sensorDisplayPosition = (sensorDisplayPosition + 1) % SENSORS_DISPLAY_WIDTH;

                    buffer[(index)++] = 0;
                    putstr(COM2, buffer);
                }
            }

        }
    }
}

void LocationOffsetDisplayTask() {
    char buffer[200] = {0};
    int index = 0;

    int row, col, offset, sensor;
    char sensor_str[20] = {0};

    char msg[10] = {0}, rpl[10] = {0};
    int msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = rpl;

    int sender_tid;
    Receive( &sender_tid, (char*)&msg_struct, msglen );
    Reply (sender_tid, (char *)&reply_struct, rpllen);

    int train_id = msg_struct.iValue;
    int row_offset = msg_struct.value[0];

    FOREVER {
        index = 0;
        Delay(20);
        getTrainLocation(train_id, &sensor, &offset);

        bwi2a( (int)(offset/10), sensor_str);
        row = TRAIN_TABLE_X + row_offset; col = OFFSET_POSITION_Y;
        outputPutStr ( "         ", &row, &col , buffer, &index );
        row = TRAIN_TABLE_X + row_offset; col = OFFSET_POSITION_Y;
        outputPutStr ( sensor_str, &row, &col , buffer, &index );

        buffer[(index)++] = 0;
        putstr(COM2, buffer);
    }
}

void LocationDisplayTask() {
    char buffer[600] = {0};
    int index = 0;

    int row, col;
    int train_info[10] = {-1};
    char sensor_str[20] = {0};
    int sender_tid, offset_tid;

    char msg[10] = {0}, rpl[10] = {0};
    int msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = rpl;

    Receive( &sender_tid, (char*)&msg_struct, msglen );
    Reply (sender_tid, (char *)&reply_struct, rpllen);

    offset_tid = Create(6, &LocationOffsetDisplayTask);
    Send (offset_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);

    int train_id = msg_struct.iValue;
    int row_offset = msg_struct.value[0];

    bwi2a( train_id, sensor_str);
    row = TRAIN_TABLE_X + row_offset; col = 1;
    outputPutStr ( sensor_str, &row, &col , buffer, &index );
    buffer[(index)++] = 0;
    putstr(COM2, buffer);

    int last_prev=0, last_next=0;

    FOREVER {
        waitTrainInfo(train_id, train_info);
        int index = 0;

        cursorCommand ( "[31m" , buffer, &index ); // set color to white
        get_cordinates((int)last_prev, &row, &col);
        row+=TRACK_POSITION_X-1;
        col+=TRACK_POSITION_Y-1;
        outputPutStr ( "o", &row, &col, buffer, &index );

        get_cordinates((int)last_next, &row, &col);
        row+=TRACK_POSITION_X-1;
        col+=TRACK_POSITION_Y-1;
        outputPutStr ( "o", &row, &col, buffer, &index );
        cursorCommand ( "[0m" , buffer, &index ); // set color to white

        last_next = (int)train_info[1];
        sensor_str[0] = 'A' + (train_info[1] / 16);
        bwi2a( (train_info[1] % 16) + 1, sensor_str + 1);
        row = TRAIN_TABLE_X + row_offset; col = NEXT_POSITION_Y;
        outputPutStr ( "      ", &row, &col , buffer, &index );
        row = TRAIN_TABLE_X + row_offset; col = NEXT_POSITION_Y;
        outputPutStr ( sensor_str, &row, &col , buffer, &index );

        get_cordinates((int)train_info[1], &row, &col);
        row+=TRACK_POSITION_X-1;
        col+=TRACK_POSITION_Y-1;
        cursorCommand ( "[36;1m" , buffer, &index ); // set color to white
        outputPutStr ( "O", &row, &col, buffer, &index );
        cursorCommand ( "[0m" , buffer, &index ); // set color to white


        last_prev = (int)train_info[0];
        sensor_str[0] = 'A' + (train_info[0] / 16);
        bwi2a( (train_info[0] % 16) + 1, sensor_str + 1);
        row = TRAIN_TABLE_X + row_offset; col = PREV_POSITION_Y;
        outputPutStr ( "      ", &row, &col , buffer, &index );
        row = TRAIN_TABLE_X + row_offset; col = PREV_POSITION_Y;
        outputPutStr ( sensor_str, &row, &col , buffer, &index );

        get_cordinates((int)train_info[0], &row, &col);
        row+=TRACK_POSITION_X-1;
        col+=TRACK_POSITION_Y-1;
        cursorCommand ( "[32;1m" , buffer, &index ); // set color to white
        outputPutStr ( "O", &row, &col, buffer, &index );
        cursorCommand ( "[0m" , buffer, &index ); // set color to white


        sensor_str[0] = 'A' + (train_info[4] / 16);
        bwi2a( (train_info[4] % 16) + 1, sensor_str + 1);
        row = TRAIN_TABLE_X + row_offset; col = DEST_POSITION_Y;
        outputPutStr ( "      ", &row, &col , buffer, &index );
        row = TRAIN_TABLE_X + row_offset; col = DEST_POSITION_Y;
        outputPutStr ( sensor_str, &row, &col , buffer, &index );

        row = TRAIN_TABLE_X + row_offset; col = EXPECTED_POSITION_Y;
        outputPutStr ( "           ", &row, &col , buffer, &index );
        row = TRAIN_TABLE_X + row_offset; col = EXPECTED_POSITION_Y;
        formClockStr(train_info[2], sensor_str);
        outputPutStr ( sensor_str, &row, &col , buffer, &index );

        row = TRAIN_TABLE_X + row_offset; col = ACTUAL_POSITION_Y;
        outputPutStr ( "           ", &row, &col , buffer, &index );
        row = TRAIN_TABLE_X + row_offset; col = ACTUAL_POSITION_Y;
        formClockStr(train_info[3], sensor_str);
        outputPutStr ( sensor_str, &row, &col , buffer, &index );

        buffer[(index)++] = 0;
        putstr(COM2, buffer);
    }
}

void initTrainDisplay(int train_id, int offset) {
    char msg[10] = {0}, rpl[10] = {0};
    int msglen = 10, rpllen = 10;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = rpl;

    int display_tid = Create (5, &LocationDisplayTask);
    msg_struct.iValue = train_id;
    msg_struct.value[0] = (char)offset;
    Send (display_tid, (char *)&msg_struct, msglen, (char *)&reply_struct, rpllen);
}

void handleCommandTask() {

    int tempi, receiver_tid;

    char buffer[1024] = {0};
    int index = 0;

    char commandStr[64];
    int command_str_index=0, row, col;
    char c;

    char msg[10];
    char reply[10];
    int i;
    message msg_struct, reply_struct;
    msg_struct.value = msg;
    reply_struct.value = reply;

    // for sensor predicting
    char predict_result[10] = {0};
    int cur_sensor;
    int prediction_len, result;
    char tempstr[20] = {0};

    // for listing reserved nodes
    char reserved_nodes[80] = {0};

    buffer[index++] = 0;
    putstr(COM2, buffer);

    Create(3, (&TracksTask));
    putc(COM1, 0x60);
    Delay(100);

    // paht finding shits
    int stopping_sensor;            // returning node
    int stoppong_sensor_dist;       // returning distance
    char sensor_route[180] = {0};    // the sensors the train's gonna
    int blocked_nodes[TRACK_MAX] = {0};
    int nodes[TRACK_MAX] = {0};

    int train_display_offset = 1;
    setAllTrainSpeedToOne();
    // set all train speed to 1 before we do anything
    index = 0;
    setCursor( CMD_POSITION_X, CMD_POSITION_Y, buffer, &index);
    buffer[(index)++] = 0;
    putstr(COM2, buffer);

    for ( i=1; i <=18 ; i++) {
        // Delay(5);
        setSwitchForce ( SW_STRAIGHT, i, true);
    }
    // setSwitch ( SW_CURVE, 3);
    // setSwitch ( SW_CURVE, 7);
    // setSwitch ( SW_CURVE, 14);
    // setSwitch ( SW_CURVE, 18);

    setSwitchForce ( SW_STRAIGHT, 0x99, true);
    setSwitchForce ( SW_CURVE, 0x99, true);
    setSwitchForce ( SW_STRAIGHT, 0x9A, true);
    setSwitchForce ( SW_STRAIGHT, 0x9B, true);
    setSwitchForce ( SW_CURVE, 0x9B, true);
    setSwitchForce ( SW_STRAIGHT, 0x9C, true);

    // Delay(600);
    Create(3, (&SensorServer));
    Create(6, (&SensorsDisplayTask));
    Create(7, (&IdleDisplayTask));

    Create(3, (&CommandCenterServer));
    // Create(5, (&LocationDisplayTask));
    // Create(5, (&LocationOffsetDisplayTask));
    // initTrainLocation(49, 0 );

    move_data md;

    FOREVER {
        c = getc(COM2);
        if (c == '\r') {
            commandStr[command_str_index++] = 0;
            command_str_index = 0;

            int command = -1;
            char argv[10][10];
            int argc;
            parseCommand( commandStr, &argc , argv, &command );
            row = CMD_POSITION_X-1; col = 1;
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
                        row = CMD_POSITION_X-1; col = 1;
                        outputPutStrLn (tempstr, &row, &col, buffer, &index );
                    }
                    break;
                case CMD_FIND_DISTANCE:
                    result = findDistanceBetweenLandmarks(atoi(argv[0]), atoi(argv[1]), TRACK_MAX);

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
                        initTrainDisplay(atoi(argv[0]), train_display_offset);
                        train_display_offset++;
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
                        outputPutStr ( argv[3], &row, &col, buffer, &index );
                        outputPutStr ( "cm", &row, &col, buffer, &index );
                        outputPutStr ( " ahead of ", &row, &col, buffer, &index );
                        outputPutStr ( argv[1], &row, &col, buffer, &index );
                        outputPutStr ( argv[2], &row, &col, buffer, &index );
                        setTrainDestination(atoi(argv[0]), result, atoi(argv[3]) );
                    }
                    break;
                case CMD_TRAIN_DEST_MULT:
                    for (i = 1; i < argc-1; i+=2) {
                        result = sensorToInt(argv[i][0], atoi(argv[i+1]));
                        if (result < 0 || result > 79) {
                            DebugPutStr("scd", "Invalid Sensor ", argv[i][0], atoi(argv[i+1]));
                        }
                        else {
                            DebugPutStr("sdscd", "Queueing Train ", atoi(argv[0]), " to dest ", argv[i][0], atoi(argv[i+1]));
                            setTrainDestination(atoi(argv[0]), result, 0 );
                        }
                    }
                    break;
                case CMD_PATH_FIND:
                    stopping_sensor = -1;
                    stoppong_sensor_dist = -1;
                    result = pathFind(
                        atoi(argv[0]),          // current node
                        atoi(argv[1]),          // where it wants to go
                        1090,                     // stoping distance
                        &stopping_sensor,       // returning node
                        &stoppong_sensor_dist,  // returning distance
                        sensor_route           // the sensors the train's gonna pass
                    );

                    // if (result >= 0) {
                        bwi2a(stopping_sensor, tempstr);
                        row = CMD_POSITION_X-1; col = 1;
                        outputPutStrLn (tempstr, &row, &col, buffer, &index );
                        bwi2a(stoppong_sensor_dist, tempstr);
                        row = CMD_POSITION_X-1; col = 1;
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
                case CMD_PF_DIJKSTRA:
                    stopping_sensor = -1;
                    stoppong_sensor_dist = -1;

                    memset(&md, 0, sizeof(move_data));
                    blocked_nodes[0] = 0;
                    int blocked_nodes_len =0;
                    result = pathFindDijkstra(
                        &md,
                        atoi(argv[0]),          // current node
                        atoi(argv[2]),          // offset
                        atoi(argv[1]),          // where it wants to go
                        400,                   // stoping distance
                        blocked_nodes,          // the sensors the train's gonna pass
                        blocked_nodes_len,
                        2
                    );

                    if (md.type == SHORT_MOVE) {
                        DebugPutStr("sd", "SHORT_MOVE ", md.total_distance);
                    }
                    else if (md.type == LONG_MOVE) {
                        DebugPutStr("sdsd", "LONG_MOVE ", md.stopping_sensor, " | ", md.stopping_dist);
                    }
                    else if (md.type == PATH_NOT_FOUND) {
                        DebugPutStr("s", "PATH_NOT_FOUND");
                        break;
                    }
                    if (md.unsafe_forward) {
                        DebugPutStr("s", "unsafe forward");
                    }

                    if (md.reverse_first) {
                        col = 1;
                        DebugPutStr("s", "reverse first");
                    }

                    for (i = 0; i < md.list_len; i++) {
                        if (md.node_list[i].type == NODE_SENSOR) {
                            DebugPutStr("sd", "sensor: ", md.node_list[i].num);
                        }
                        else if (md.node_list[i].type == NODE_BRANCH) {
                            DebugPutStr("sdsd", "branch: ", md.node_list[i].num, " | ", md.node_list[i].branch_state);
                        }
                        else if (md.node_list[i].type == NODE_MERGE) {
                            DebugPutStr("sd", "merge: ", md.node_list[i].num);
                        }
                    }
                    DebugPutStr("sd", "list_len: ", md.list_len);
                    break;
                case CMD_SHORT_MOVE_TIME:
                    // sm <train num> <time>
                    setTrainSpeed( atoi(argv[0]), 12);
                    Delay(atoi(argv[1]));
                    setTrainSpeed( atoi(argv[0]), 0);
                    break;
                case CMD_INIT_TRACK:
                    if (argv[0][0] == 'a') {
                        DebugPutStr("s", "Init Track to A");
                    }
                    else {
                        DebugPutStr("s", "Init Track to B");
                    }
                    initTrack(argv[0][0]);
                    break;
                case CMD_SET_SHORT_MULT:
                    setTrainShortMult( atoi(argv[0]), atoi(argv[1]));
                    break;
                case CMD_SET_STOPPING_DIST:
                    setTrainStopDist( atoi(argv[0]), atoi(argv[1]));
                    break;
                case CMD_RESERVE_NODE:
                    DebugPutStr("s", "Reseving Nodes:");
                    for (i = 0; i < argc - 1; i+= 2) {
                        result = sensorToInt(argv[i][0], atoi(argv[i+1]));
                        DebugPutStr("cdsd", argv[i][0], atoi(argv[i+1]), " ", result);
                        argv[0][i/2] = result;
                    }

                    reserveNodesRequest(argv[0], argc / 2, 1);
                    break;
                case CMD_FREE_NODE:
                    DebugPutStr("s", "Freeing Nodes:");
                    for (i = 0; i < argc - 1; i+= 2) {
                        result = sensorToInt(argv[i][0], atoi(argv[i+1]));
                        DebugPutStr("cdsd", argv[i][0], atoi(argv[i+1]), " ", result);
                        argv[0][i/2] = result;
                    }

                    freeNodes(argv[0], argc / 2, 1);
                    break;
                case CMD_QUIT:
                    // train_buffer[train_rindex % BUFFER_SIZE] = 0x61;
                    // train_rindex++;
                    outputPutStrLn ( "Qutting", &row, &col, buffer, &index );
                    break;
                case CMD_CREATE_SENSOR_NOTIFIER:
                    outputPutStrLn ( "Creating new Notifier for Sensor", &row, &col, buffer, &index );
                    msg_struct.type = SENSORS_CREATE_NOTIFIER;
                    receiver_tid = WhoIs(SENSOR_SERVER_NAME);
                    Send (receiver_tid, (char *)&msg_struct, 0, (char *)&reply_struct, 20);
                    break;
                case CMD_LIST_RESERVED_NODE:
                    result = getReservedNodes(reserved_nodes, 80);
                    DebugPutStr("ds", result, "Reserved Nodes:");
                    for (i = 0; i < result; i++) {
                        DebugPutStr("cd", reserved_nodes[i]/16+'A', reserved_nodes[i]%16+1);
                    }
                    break;
                case CMD_NEXT_SENSORS_CHECK:
                    result = nextPossibleSensorsCheck(nodes, 20, atoi(argv[0]), atoi(argv[1]), atoi(argv[2]), &tempi);
                    if (result == true) {
                        DebugPutStr("s", "Can Move");
                    }
                    else {
                        DebugPutStr("s", "Don't Move");
                    }
                    for (i = 0; i < tempi; i++) {
                        DebugPutStr("cd", nodes[i]/16+'A', nodes[i]%16+1);
                    }
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
        else if (c == 0x08) {
            if (command_str_index > 0){
                command_str_index--;
                index = 0;
                setCursor( CMD_POSITION_X, CMD_POSITION_Y+command_str_index, buffer, &index);
                flushLine (buffer, &index);
                buffer[(index)++] = 0;
                putstr(COM2, buffer);
            }

        }
    	else {
    		putc(COM2, c);
    		commandStr[command_str_index++] = c;
    	}
    }
}
