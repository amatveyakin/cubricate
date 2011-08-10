const int     TREE_HEIGHT = 6;
const int     MAP_SIZE = 1 << TREE_HEIGHT;
const int     N_MAP_BLOCKS = MAP_SIZE * MAP_SIZE * MAP_SIZE;

const int     NODE_STRUCT_SIZE = 6;

// Node structure: Type | NeigbourX | NeigbourY | NeigbourZ | Height | Parameter
const int     NODE_OFFSET_TYPE       = 0;
const int     NODE_OFFSET_HEIGHT     = 4;
const int     NODE_OFFSET_NEIGHBOURS = 0;
const int     NODE_OFFSET_PARAMETER  = 5;

const int     SCREEN_WIDTH  = 1024;
const int     SCREEN_HEIGHT = 1024;

const int     RAY_PACKET_WIDTH  = 4;
const int     RAY_PACKET_HEIGHT = 4;

const int     MAX_FLUID_SATURATION = 64;
