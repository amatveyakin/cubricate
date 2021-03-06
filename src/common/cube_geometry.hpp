#ifndef CUBE_GEOMETRY_HPP
#define CUBE_GEOMETRY_HPP


#include <stdexcept>

#include "common/c++11_feature_tests.hpp"
#include "common/game_parameters.hpp"
#include "common/linear_algebra.hpp"
#include "common/string_utils.hpp"


//   Numbering and naming convetions
//  ---------------------------------
//
//              up
//                    front
//            +------+                            6+------+7
//           /|     /|                            /|     /|
//          +------+ |                          4+------+5|
//    left  | |    | |  right                    | |    | |
//          | +----|-+                           |2+----|-+3
//          |/     |/                            |/     |/
//          +------+                            0+------+1
//      back
//             down
// standard names for cube faces       standard cube vertex numbering
//
//
// These face names are not very much correct 'cause they even do not
// belong to the same part of speech. But these names have that advantage
// that their first letter differ, so they can be used of abbreviations
// (e.g. ``ufr'' corner for the upper front right one)
//
// NOTE: ``front'' and ``back'' faces are NOT mixed up. Oh, well, they are
// but it is done intentionally 'cause these word first of all stand for
// world directions (look at the picture on the right).
// That it certainly quite confusing but using                up
// different front--back notation for cube faces              ^    front
// and world direction (while left--right                     |   ^
// and up--down pairs' meaning match)                         |  /
// will definitely be even more confusing,                    | /
// so when you this of a face of a cube                       |/
// you shouldn't imagine that you hold            <-----------+----------->
// the cube in front of you but instead         left         /|         right
// you should think that you ARE the cube.                  / |
// That should make the things clear.                      /  |
//                                                        v   |
//                                                     back   v
//                                                           down



// TODO: fix the following text, it doen't correspond the code now

//   World floating-point coordinates and cube integer coordinates relationship
//  ----------------------------------------------------------------------------
//
//            +------+
//           /|     /|
//          +------+ |
//          | | . <----- (x, y, z) point
//          | +----|-+
//          |/     |/
//          +------+
//                  (x, y, z) cube
//
//
//
//       #################################################
//    7  #   |   |   |   #   |   |   |   #   |   |   |   #
//       #___|___|___|___#___|___|___|___#___|___|___|___#
//    6  # (-1,1) chunk  #  (0,1) chunk  #  (1,1) chunk  #
//       #___|___|___|___#___|___|___|___#___|___|___|___#
//    5  #   |   |   |   #   |   |   |   #   |   |   |   #
//       #___|___|___|___#___|___|___|___#___|___|___|___#
//    4  #   |   |   |   #   |   |   |   #   |   |   |   #
//       #   |   |   |   #   |   |   |   #   |   |   |   #
//       #################################################
//    3  #   |   |   |   #   |   |   |   #   |   |   |   #
//       #___|___|___|___#___|___|___|___#___|___|___|___#
//    2  # (-1,0) chunk  #  (0,0) chunk  #  (1,0) chunk  #
//       #___|___|___|___#___|___|___|___#___|___|___|___#
//    1  #   |   |   |   #   |   |   |   #   |   |   |   #
//       #___|___|___|___#___|___|___|___#___|___|___|___#
//    0  #   |   |   |   # .<--------------------------------- the origin of
//       #   |   |   |   #   |   |   |   #   |   |   |   #     the world coordinates
//       #################################################
//   -1  #   |   |   |   #   |   |   |   #   |   |   |   #
//       #___|___|___|___#___|___|___|___#___|___|___|___#
//   -2  # (-1,-1) chunk # (0,-1) chunk  # (1,-1) chunk  #
//       #___|___|___|___#___|___|___|___#___|___|___|___#
//   -3  #   |   |   |   #   |   |   |   #   |   |   |   #
//       #___|___|___|___#___|___|___|___#___|___|___|___#
//   -4  #   |   |   |   #   |   |   |   #   |   |   |   #
//       #   |   |   |   #   |   |   |   #   |   |   |   #
//       #################################################
//
//        -4  -3  -2  -1   0   1   2   3   4   5   6   7


#if CPP11_ENUM_ENCHANTMENTS
enum class Direction {
#else // !CPP11_ENUM_ENCHANTMENTS
enum Direction {
#endif // !CPP11_ENUM_ENCHANTMENTS
  X_PLUS,
  Y_PLUS,
  Z_PLUS,
  X_MINUS,
  Y_MINUS,
  Z_MINUS,
  INVALID
};

const int N_DIRECTIONS = 6;

static inline bool directionIsValid (int dir) {
  return dir >= 0 && dir < N_DIRECTIONS;
}

static inline bool directionIsValid (Direction dir) {
  return directionIsValid (int (dir));
}

const Direction DIRECTION_BEGIN = Direction::X_PLUS;
const Direction DIRECTION_END   = Direction::INVALID;

static inline Direction operator++ (Direction dir) {
  dir = static_cast <Direction> (static_cast <int> (dir) + 1);
  return dir;
}

// struct CubeWithFace : Vec3i {
//   CubeWithFace (Vec3i cube__, Direction face__) : Vec3i (cube__), face (face__) { }
//
//   const Vec3i& cube () const  { return static_cast <const Vec3i&> (*this); }
//   Vec3i& cube ()              { return static_cast <Vec3i&> (*this); }
//
//   Direction face;
// };

struct CubeWithFace {
  Vec3i cube;
  Direction face;

  CubeWithFace (Vec3i cube__, Direction face__) : cube (cube__), face (face__) { }
};


// TODO: rename ``cube'' -> ``block'' (?)

// TODO: write other variants for these functions

static inline void getUnifiedCube (/* i/o */ int& x, int& y, int& z, Direction& face) {
  switch (face) {
    case Direction::X_PLUS:
    case Direction::Y_PLUS:
    case Direction::Z_PLUS:
      return;
    case Direction::X_MINUS:
      x--;
      face = Direction::X_PLUS;
      return;
    case Direction::Y_MINUS:
      y--;
      face = Direction::Y_PLUS;
      return;
    case Direction::Z_MINUS:
      z--;
      face = Direction::Z_PLUS;
      return;
    case Direction::INVALID:
      break;
  }
  throw std::invalid_argument ("Bad face type: " + toStr (int (face)));
}

static inline CubeWithFace getUnifiedCube (CubeWithFace cubeWithFace) {
  getUnifiedCube (cubeWithFace.cube.x (), cubeWithFace.cube.y (), cubeWithFace.cube.z (), cubeWithFace.face);
  return cubeWithFace;
}


static inline void getAdjacentCube (/* i/o */ int& x, int& y, int& z, Direction& face) {
  switch (face) {
    case Direction::X_PLUS:
      x++;
      face = Direction::X_MINUS;
      return;
    case Direction::Y_PLUS:
      y++;
      face = Direction::Y_MINUS;
      return;
    case Direction::Z_PLUS:
      z++;
      face = Direction::Z_MINUS;
      return;
    case Direction::X_MINUS:
      x--;
      face = Direction::X_PLUS;
      return;
    case Direction::Y_MINUS:
      y--;
      face = Direction::Y_PLUS;
      return;
    case Direction::Z_MINUS:
      z--;
      face = Direction::Z_PLUS;
      return;
    case Direction::INVALID:
      break;
  }
  throw std::invalid_argument ("Bad face type: " + toStr (int (face)));
}

static inline Vec3i getAdjacentCube (Vec3i cube, Direction face) {
  getAdjacentCube (XYZ_LIST (cube), face);
  return cube;
}

static inline CubeWithFace getAdjacentCube (CubeWithFace cubeWithFace) {
  getAdjacentCube (XYZ_LIST (cubeWithFace.cube), cubeWithFace.face);
  return cubeWithFace;
}

static inline Direction getAdjacentFace (Vec3i cube, Vec3i neighbour) {
  Vec3i direction = neighbour - cube;
  if (L1::norm (direction) != 1)
    return Direction::INVALID;

  if      (direction.x () == 1)
    return Direction::X_PLUS;
  else if (direction.y () == 1)
    return Direction::Y_PLUS;
  else if (direction.z () == 1)
    return Direction::Z_PLUS;
  else if (direction.x () == -1)
    return Direction::X_MINUS;
  else if (direction.y () == -1)
    return Direction::Y_MINUS;
  else if (direction.z () == -1)
    return Direction::Z_MINUS;

  return Direction::INVALID;
}

// TODO: Decide whether cube centers or cube corners have integer coordinates.
static inline Vec3i worldToCube (Vec3d pos) {
//   return Vec3i::fromVectorConverted (floor (pos + Vec3d (0.5, 0.5, 0.5)));
  return Vec3i (floor (pos));
}

static inline Vec3d getCubeCenter (Vec3i cube) {
  return Vec3d (cube) + Vec3d (0.5, 0.5, 0.5);
}


// TODO: delete
static inline Vec3i getCubeByPoint (Vec3d point, Vec3d direction) {
  return worldToCube (point + direction * 0.0001);
}

static inline Vec2i cubeToChunk (Vec3i cube) {
  return cube.xy ().divFloored (CHUNK_SIZE);
}

static inline void cubeToChunk (/* in */ Vec3i cube, /* out */ Vec2i& chunk, Vec3i& cubeInChunk) {
  chunk       = cube.xy ().divFloored (CHUNK_SIZE);
  cubeInChunk = Vec3i (modFloored (cube.x (), CHUNK_SIZE), modFloored (cube.y (), CHUNK_SIZE), cube.z ());
}

static inline Vec2i worldToChunk (Vec3d pos) {
  return cubeToChunk (worldToCube (pos));
}

static inline void worldToChunk (/* in */ Vec3d pos, /* out */ Vec2i& chunk, Vec3i& cubeInChunk) {
  Vec3i cube = worldToCube (pos);
  cubeToChunk (cube, chunk, cubeInChunk);
}



// A cube that is guaranteed to be invalid
static const Vec3i INVALID_CUBE (-1, -1, -1);

static inline bool cubeIsValid (int x, int y, int z) {
  return    (x >= 0) && (x < MAP_SIZE)
         && (y >= 0) && (y < MAP_SIZE)
         && (z >= 0) && (z < MAP_SIZE);
}

static inline bool cubeIsValid (Vec3i v) {
  return cubeIsValid (v.x (), v.y (), v.z ());
}


#endif
