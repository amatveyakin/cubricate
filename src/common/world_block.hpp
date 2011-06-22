#ifndef WORLD_BLOCK_HPP
#define WORLD_BLOCK_HPP


enum class BlockType : unsigned char {
  EMPTY = 0,
  DIRT,
  SNOW,
  LAVA
};


struct WorldBlock {
  BlockType type;
  union {
    struct {
      unsigned char attachedTo : 3;
    };
    unsigned char parameters;
  };

  WorldBlock () = default;
  WorldBlock (BlockType type__) {
    type = type__;
    parameters = 0;
  }
};


static_assert (sizeof (WorldBlock) == sizeof (unsigned char [2]), "Is this assertion fails, it's a good reason to check that WorldBlock::parameters didn't become too fat.");


#endif
