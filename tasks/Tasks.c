#include <Tasks.h>
#include <bwio.h>

void FirstUserTask (){
    int tid = Create(3, CODE_OFFSET + (&spawnedTask));
    bwprintf(COM2, "Created: %d\n", tid);

    tid = Create(3, CODE_OFFSET + (&spawnedTask));
    bwprintf(COM2, "Created: %d\n", tid);

    tid = Create(1, CODE_OFFSET + (&spawnedTask));
    bwprintf(COM2, "Created: %d\n", tid);

    tid = Create(1, CODE_OFFSET + (&spawnedTask));
    bwprintf(COM2, "Created: %d\n", tid);

    bwprintf(COM2, "First: Exiting\n" );
    Exit();
}

void spawnedTask () {
    int tid = MyTid();
    int ptid = MyParentTid();
    bwprintf(COM2, "Spawned task tid: %d\n", tid);
    bwprintf(COM2, "Spawned parent task tid: %d\n", ptid);
    Pass();
    bwprintf(COM2, "Spawned task tid: %d\n", tid);
    bwprintf(COM2, "Spawned parent task tid: %d\n", ptid);
    Exit();
}
