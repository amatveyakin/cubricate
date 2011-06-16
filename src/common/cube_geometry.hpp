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
  DOWN
};

const int N_DIRECTIONS = 6;


struct CubeWithFace : Vec3i {
  const Vec3i& cube () const  { return static_cast <const Vec3i&> (*this); }
  Vec3i& cube ()              { return static_cast <Vec3i&> (*this); }
  Direction face;
};


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
  }
  throw std::invalid_argument ("Bad face type: " + toStr (int (face)));
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
  }
  throw std::invalid_argument ("Bad face type: " + toStr (int (face)));
}


static inline Vec3i worldToCube (Vec3d pos) {
  return Vec3i::fromVectorConverted (pos + Vec3d (0.5, 0.5, 0.5));
}

static inline Vec3i cubeToChunk (Vec3i cube) {
  return cube.divFloored (CHUNK_SIZE);
}

static inline void cubeToChunk (/* in */ Vec3i cube, /* out */ Vec3i& chunk, Vec3i& cubeInChunk) {
  // TODO: implement a combined divModFloored function
  chunk       = cube.divFloored (CHUNK_SIZE);
  cubeInChunk = cube.modFloored (CHUNK_SIZE);
}

static inline Vec3i worldToChunk (Vec3d pos) {
  return cubeToChunk (worldToCube (pos));
}

static inline void worldToChunk (/* in */ Vec3d pos, /* out */ Vec3i& chunk, Vec3i& cubeInChunk) {
  Vec3i cube = worldToCube (pos);
  cubeToChunk (cube, chunk, cubeInChunk);
}

#endif
