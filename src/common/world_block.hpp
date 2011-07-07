#ifndef WORLD_BLOCK_HPP
#define WORLD_BLOCK_HPP


enum BlockType : unsigned char {
  // Gases
  BT_AIR = 0,

  // Liquids
  BT_WATER,

  // Semitransparent solids
//   BT_GLASS,

  // Opaque solids
  BT_DIRT,
  BT_GRASS,
  BT_SNOWY_DIRT,
  BT_BRICKS
};

const int N_BLOCK_TYPES = BT_BRICKS + 1;


namespace BlockInfo {

  namespace BlockInfoPrivate {
    const BlockType firstGas                  = BT_AIR;
    const BlockType firstLiquid               = BT_WATER;
    const BlockType firstSemitransparentSolid = BT_DIRT;
    const BlockType firstOpaqueSolid          = BT_DIRT;
  }


  static inline bool isGas (BlockType type) {
    return /*BlockInfoPrivate::firstGas <= type &&*/ type < BlockInfoPrivate::firstLiquid;
  }

  static inline bool isLiquid (BlockType type) {
    return BlockInfoPrivate::firstLiquid <= type && type < BlockInfoPrivate::firstSemitransparentSolid;
  }

  static inline bool isSolid (BlockType type) {
    return BlockInfoPrivate::firstSemitransparentSolid <= type && type < N_BLOCK_TYPES;
  }


  static inline bool isSemitransparent (BlockType type) {
    return /*BlockInfoPrivate::firstGas <= type &&*/ type < BlockInfoPrivate::firstOpaqueSolid;
  }

  static inline bool isOpaque (BlockType type) {
    return BlockInfoPrivate::firstOpaqueSolid <= type;
  }

}


struct WorldBlock {
  BlockType type;
  union {
    // For objects, attached to adjacent cube
    struct {
      unsigned char attachedTo : 3;
    };

    // For liquids
    float fluidSaturation;

    // All parameters as an integer
    unsigned int parameters;
  };
  static_assert (sizeof (fluidSaturation) == sizeof (parameters), "");


  WorldBlock () = default;

  WorldBlock (BlockType type__) {
    type = type__;
    switch (type) {
      case BT_AIR:
      case BT_DIRT:
      case BT_GRASS:
      case BT_SNOWY_DIRT:
      case BT_BRICKS:
        parameters = 0;

      case BT_WATER:
        fluidSaturation = 1.;
    }
  }
};


// static_assert (sizeof (WorldBlock) == sizeof (unsigned char [2]), "Is this assertion fails, it's a good reason to check that WorldBlock::parameters didn't become too fat.");


#endif
