#include <iostream>


int abs (int x) {
  return (x < 0) ? -x : x;
}


const int N_DIRECTIONS          = 7;
const int SPACE_DIMENSION       = 3;
const int N_TREE_NODE_CHILDREN  = 8;

// const int directions[N_DIRECTIONS][3] = {
//   {  0,  0, -1 },
//   {  0, -1,  0 },
//   { -1,  0,  0 },
//   {  1,  0,  0 },
//   {  0,  1,  0 },
//   {  0,  0,  1 }
// };

// Order:  NeigbourZ- | NeigbourY- | NeigbourX- | NeigbourX+ | NeigbourY+ | NeigbourZ+

const int DIRECTIONS[N_DIRECTIONS] = {
  -0x4,
  -0x2,
  -0x1,
   0x0,
   0x1,
   0x2,
   0x4
};


int main (int argc, char **argv) {
  std::cout << "int putYourFavoriteArrayNameHere[" << N_TREE_NODE_CHILDREN << "][" << N_DIRECTIONS << "] = {\n";
  std::cout << "  /* Z-  Y-  X-      X+  Y+  Z+ */\n";
  for (int iChild = 0; iChild < N_TREE_NODE_CHILDREN; ++iChild) {
    std::cout << "  { ";
    for (int iDir = 0; iDir < N_DIRECTIONS; ++iDir) {
      std::cout.width (2);
      int dir = DIRECTIONS [iDir];
      if (  ((iChild & abs (dir)) != 0)  // We can move is negative direction
          ^ (dir > 0)                  ) // Current direction is positive
        std::cout << (iChild - (iChild ^ abs (dir)));
      else
        std::cout << 0;
      std::cout << ((iDir < N_DIRECTIONS - 1) ? ", " : "  ");
    }
    std::cout << "}" << ((iChild < N_TREE_NODE_CHILDREN - 1) ? "," : " ") << "  /* " << iChild << " */\n";
  }
  std::cout << "};\n";
  return 0;
}
