#ifndef WORLD_BLOCK_HPP
#define WORLD_BLOCK_HPP


enum class BlockType : unsigned char {
  AIR = 0,
  DIRT,
  SNOW,
  LAVA
};


struct WorldBlock {
  BlockType type;
  union {
    struct {
      unsigned int attachedTo : 3;
    };
    unsigned char parameters;
  };
};


#endif
