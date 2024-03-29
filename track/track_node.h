#ifndef __TRACK_NODE__
#define __TRACK_NODE__

typedef enum {
  NODE_NONE,
  NODE_SENSOR,
  NODE_BRANCH,
  NODE_MERGE,
  NODE_ENTER,
  NODE_EXIT,
} node_type;

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1

struct track_node;
typedef struct track_node track_node;
typedef struct track_edge track_edge;

struct track_edge {
  track_edge *reverse;
  track_node *src, *dest;
  int dist;             /* in millimetres */
};

struct track_node {
  const char *name;
  node_type type;
  int num;              /* sensor or switch number */
  int index;
  track_node *reverse;  /* same location, but opposite direction */
  track_node *parent;
  int visisted;
  int reserved[5];
  track_edge edge[2];
  track_node *binded_nodes[2];
};

#endif
