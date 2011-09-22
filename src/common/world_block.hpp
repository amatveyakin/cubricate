#ifndef WORLD_BLOCK_HPP
#define WORLD_BLOCK_HPP


#include <cassert>

#include "common/c++11_feature_tests.hpp"

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
  BT_BRICKS,
  BT_MIRROR,

  BT_TEST_LIGHT,

  BT_TEST_SUBOBJECT,


  BT_INVALID
};

const int N_BLOCK_TYPES = BT_INVALID;


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
#if CPP11_CLASS_STATIC_ASSERTS
  static_assert (sizeof (fluidSaturation) == sizeof (parameters), "");
#endif // CPP11_CLASS_STATIC_ASSERTS


  WorldBlock() { }

  WorldBlock (BlockType type__) {
    type = type__;
    switch (type) {
      case BT_AIR:
      case BT_DIRT:
      case BT_GRASS:
      case BT_SNOWY_DIRT:
      case BT_BRICKS:
      case BT_MIRROR:
      case BT_TEST_SUBOBJECT:
      case BT_TEST_LIGHT:
        parameters = 0;
        break;

      case BT_WATER:
        fluidSaturation = 1.;
        break;

      case BT_INVALID:
        assert (false);
        break;
    }
  }
};


// static_assert (sizeof (WorldBlock) == sizeof (unsigned char [2]), "Is this assertion fails, it's a good reason to check that WorldBlock::parameters didn't become too fat.");


// TODO: use masks instead (?)
namespace BlockInfo {

  const BlockType firstGas                  = BT_AIR;
  const BlockType firstLiquid               = BT_WATER;
  const BlockType firstSemitransparentSolid = BT_DIRT;
  const BlockType firstOpaqueSolid          = BT_DIRT;
  const BlockType firstSubobject            = BT_TEST_SUBOBJECT;


  static inline bool isGas (BlockType type) {
    return /*firstGas <= type &&*/ type < firstLiquid;
  }

  static inline bool isLiquid (BlockType type) {
    return firstLiquid <= type && type < firstSemitransparentSolid;
  }

  static inline bool isFluid (BlockType type) {
    return isGas (type) || isLiquid (type);
  }

  static inline bool isSolid (BlockType type) {
    return firstSemitransparentSolid <= type && type < firstSubobject;
  }

  static inline bool isSubobject (BlockType type) {
    return firstSubobject <= type && type < N_BLOCK_TYPES;
  }

  static inline bool isFirm (BlockType type) {
    return isSolid (type) || isSubobject (type);
  }


  // TODO: what about subobjects?
  static inline bool isSemitransparent (BlockType type) {
    return /*firstGas <= type &&*/ type < firstOpaqueSolid;
  }

  static inline bool isOpaque (BlockType type) {
    return firstOpaqueSolid <= type;
  }



  static inline bool isGas (WorldBlock block) {
    return isGas (block.type);
  }

  static inline bool isLiquid (WorldBlock block) {
    return isLiquid (block.type);
  }

  static inline bool isFluid (WorldBlock block) {
    return isFluid (block.type);
  }

  static inline bool isSolid (WorldBlock block) {
    return isSolid (block.type);
  }

  static inline bool isSubobject (WorldBlock block) {
    return isSubobject (block.type);
  }

  static inline bool isFirm (WorldBlock block) {
    return isFirm (block.type);
  }


  static inline bool isSemitransparent (WorldBlock block) {
    return isSemitransparent (block.type);
  }

  static inline bool isOpaque (WorldBlock block) {
    return isOpaque (block.type);
  }

}


#endif
