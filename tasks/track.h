#ifndef __TRACK__
#define __TRACK__

#include <track_data.h>
#include <track_node.h>

#define SET_SWITCH          0x50
#define PREDICT_SENSOR      0x51
#define PATH_FIND           0x52
#define FIND_DISTANCE_BETWEEN_TWO_LANDMARKS 0x53
#define PATH_FIND_DIJKSTRA  0x54
#define INIT_TRACK          0x55

#define TRACK_TASK      "track task"

#define REVERSING_WEIGHT    400
#define TRAIN_LENGTH        300

typedef enum {
    LONG_MOVE,
    SHORT_MOVE,
    SAFE_REVERSE,
    UNSAFE_REVERSE,  // have to shift the train a bit
    PATH_NOT_FOUND
} move_type;

typedef struct move_node_t {
    node_type type; // sensor, branch, or merge
    int id;         // 0 to 144
    int num;        // the actual num after the sw/A/B/C/D/E prefix
    int branch_state;
} move_node;

typedef struct path_find_requirements_t {
    int src;
    int src_node_offfset;
    int dest;
    int stopping_dist;
    int blocked_nodes[TRACK_MAX];
    int blocked_nodes_len;
} path_find_requirements;

typedef struct move_data_t {
    move_type type;
    int list_len;
    move_node node_list[TRACK_MAX];
    int stopping_sensor;
    int stopping_dist;
    int total_distance;
} move_data;

void TracksTask ();

void setSwitchStatus(unsigned int* switch_status, int sw, int dir);
int getSwitchStatus(unsigned int* switch_status, int sw);

void predictSensorTrackTask(
    track_node *tracks,     // the initialized array of tracks
    unsigned int switch_status,
    int cur_sensor,         // 0 based
    int prediction_len,     // amount of predictions wanted
    char* paths             // the triggers to be triggered
);

int findDistanceBetweenLandmarksTrackTask(
    track_node *tracks,     // the initialized array of tracks
    unsigned int switch_status,
    int landmark_start,     // 0 based
    int landmark_end,       // 0 based
    int lookup_limit        // amount of predictions wanted
);
int findDistanceBetweenLandmarks(
    int landmark1, int landmark2, int lookup_limit
);
int predictSensor( int sensor, int prediction_len, char* result );


// path shits
int pathFindTrackTask(
    track_node *tracks,     // the initialized array of tracks
    unsigned int* switch_status,
    int cur_sensor,         // 0 based
    int stopping_node,      // amount of predictions wanted
    int stopping_dist,
    int* stopping_sensor,   // the triggers to be triggered
    int* stoppong_sensor_dist,   // returning distance
    char* sensor_route
);

int bfsPathFind(        // pretty shit path find lol
    track_node *tracks,
    track_node* begin_node,
    track_node* end_node,
    track_node** path   // the path the train's gonna take
);

int pathFind(
    int cur_sensor,             // current node
    int dest_node,              // where it wants to go
    int stopping_dist,          // stoping distance
    int* stopping_sensor,       // returning node
    int* stoppong_sensor_dist,  // returning distance
    char* sensor_route          // the sensors the train's gonna pass
);

int pathFindDijkstra(
    struct move_data_t * md,
    int cur_sensor,             // current node
    int cur_offfset,            // this is the node+distance, distance
    int dest_node,              // where it wants to go
    int stopping_dist,          // stoping distance
    int blocked_items[TRACK_MAX],         // the landmarks the train cannot use
    int blocked_nodes_len
);

struct move_data_t pathFindDijkstraTrackTask(
    track_node *tracks,     // the initialized array of tracks
    unsigned int* switch_status,
    int cur_sensor,         // 0 based
    int src_node_offfset,
    int stopping_node,
    int stopping_dist,
    int blocked_nodes[TRACK_MAX],         // the landmarks the train cannot use
    int blocked_nodes_len
);

void makePath(track_node* node, track_node* init_node, track_node** path);

void setSwitchTrackTask(int switch_num, char switch_dir, unsigned int* switch_status);

void initTrack(char track);

#endif
