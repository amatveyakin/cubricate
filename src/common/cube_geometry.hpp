#ifndef CUBE_GEOMETRY_HPP
#define CUBE_GEOMETRY_HPP


#include <stdexcept>

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


enum class Direction {
  RIGHT,
  FRONT,
  UP,
  LEFT,
  BACK,
  DOWN,
  INVALID
};

const int N_DIRECTIONS = 6;

static inline bool directionIsValid (int dir) {
  return dir >= 0 && dir < N_DIRECTIONS;
}

static inline bool directionIsValid (Direction dir) {
  return directionIsValid (int (dir));
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
    case Direction::RIGHT:
    case Direction::FRONT:
    case Direction::UP:
      return;
    case Direction::LEFT:
      x--;
      face = Direction::RIGHT;
      return;
    case Direction::BACK:
      y--;
      face = Direction::FRONT;
      return;
    case Direction::DOWN:
      z--;
      face = Direction::UP;
      return;
    case Direction::INVALID:
      break;
  }
  throw std::invalid_argument ("Bad face type: " + toStr (int (face)));
}

static inline CubeWithFace getUnifiedCube (CubeWithFace cubeWithFace) {
  getUnifiedCube (cubeWithFace.cube.x, cubeWithFace.cube.y, cubeWithFace.cube.z, cubeWithFace.face);
  return cubeWithFace;
}


static inline void getAdjacentCube (/* i/o */ int& x, int& y, int& z, Direction& face) {
  switch (face) {
    case Direction::RIGHT:
      x++;
      face = Direction::LEFT;
      return;
    case Direction::FRONT:
      y++;
      face = Direction::BACK;
      return;
    case Direction::UP:
      z++;
      face = Direction::DOWN;
      return;
    case Direction::LEFT:
      x--;
      face = Direction::RIGHT;
      return;
    case Direction::BACK:
      y--;
      face = Direction::FRONT;
      return;
    case Direction::DOWN:
      z--;
      face = Direction::UP;
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

  if      (direction.x == 1)
    return Direction::RIGHT;
  else if (direction.y == 1)
    return Direction::FRONT;
  else if (direction.z == 1)
    return Direction::UP;
  else if (direction.x == -1)
    return Direction::LEFT;
  else if (direction.y == -1)
    return Direction::BACK;
  else if (direction.z == -1)
    return Direction::DOWN;

  return Direction::INVALID;
}



static inline Vec3i worldToCube (Vec3d pos) {
  return Vec3i::fromVectorConverted (floor (pos + Vec3d (0.5, 0.5, 0.5)));
}

static inline Vec2i cubeToChunk (Vec3i cube) {
  return cube.xy ().divFloored (CHUNK_SIZE);
}

static inline void cubeToChunk (/* in */ Vec3i cube, /* out */ Vec2i& chunk, Vec3i& cubeInChunk) {
  chunk       = cube.xy ().divFloored (CHUNK_SIZE);
  cubeInChunk = Vec3i (intModFloored (cube.x, CHUNK_SIZE), intModFloored (cube.y, CHUNK_SIZE), cube.z);
}

static inline Vec2i worldToChunk (Vec3d pos) {
  return cubeToChunk (worldToCube (pos));
}

static inline void worldToChunk (/* in */ Vec3d pos, /* out */ Vec2i& chunk, Vec3i& cubeInChunk) {
  Vec3i cube = worldToCube (pos);
  cubeToChunk (cube, chunk, cubeInChunk);
}



static inline bool cubeValid (int x, int y, int z) {
  return    (x >= 0) && (x < MAP_SIZE)
         && (y >= 0) && (y < MAP_SIZE)
         && (z >= 0) && (z < MAP_SIZE);
}

static inline bool cubeValid (Vec3i v) {
  return cubeValid (v.x, v.y, v.z);
}


#endif
