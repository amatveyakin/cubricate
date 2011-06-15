#ifndef CUBE_GEOMETRY_HPP
#define CUBE_GEOMETRY_HPP


#include <stdexcept>

#include "common/linear_algebra.hpp"
#include "common/string_utils.hpp"


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


enum class CubeFace {
  RIGHT,
  FRONT,
  UP,
  LEFT,
  BACK,
  DOWN
};

const int N_CUBE_FACES = 6;


typedef Vec2i CubePos;

struct CubeWithFace : CubePos {
  CubeFace face;
};


// TODO: write other variants for these functions

static inline void getUnifiedCube (/* i/o */ int& x, int& y, int& z, CubeFace& face) {
  switch (face) {
    case CubeFace::RIGHT:
      face = CubeFace::RIGHT;
      return;
    case CubeFace::FRONT:
      face = CubeFace::FRONT;
      return;
    case CubeFace::UP:
      face = CubeFace::UP;
      return;
    case CubeFace::LEFT:
      x--;
      face = CubeFace::RIGHT;
      return;
    case CubeFace::BACK:
      y--;
      face = CubeFace::FRONT;
      return;
    case CubeFace::DOWN:
      z--;
      face = CubeFace::UP;
      return;
  }
  throw std::invalid_argument ("Bad face type: " + toStr (int (face)));
}

static inline void getAdjacentCube (/* i/o */ int& x, int& y, int& z, CubeFace& face) {
  switch (face) {
    case CubeFace::RIGHT:
      x++;
      face = CubeFace::LEFT;
      return;
    case CubeFace::FRONT:
      y++;
      face = CubeFace::BACK;
      return;
    case CubeFace::UP:
      z++;
      face = CubeFace::DOWN;
      return;
    case CubeFace::LEFT:
      x--;
      face = CubeFace::RIGHT;
      return;
    case CubeFace::BACK:
      y--;
      face = CubeFace::FRONT;
      return;
    case CubeFace::DOWN:
      z--;
      face = CubeFace::UP;
      return;
  }
  throw std::invalid_argument ("Bad face type: " + toStr (int (face)));
}

#endif
