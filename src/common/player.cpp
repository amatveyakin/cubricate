#include <cassert>

#ifdef CLIENT_APP
  #include "client/client_world.hpp"
#else // !CLIENT_APP
  #include "server/server_world.hpp"
#endif // !CLIENT_APP

#include "common/player.hpp"



Player::Player () {
  m_pos = m_viewFrame.origin ();
}

Player::~Player () { }


  // TODO: delete that and rewrite normally
static const int POSITION_CHECK_ITER = 10;

void Player::setPos (Vec3d newPos) {
  m_viewFrame.setOrigin (newPos);
  m_pos = m_viewFrame.origin ();
}

void Player::moveForward (double moveBy) {
  Vec3d newPos = m_pos + m_viewFrame.dirForward () * moveBy;
  for (int i = 0; i < POSITION_CHECK_ITER; ++i)
    if (!positionIsValid ((m_pos * i + newPos * (POSITION_CHECK_ITER - i)) / double (POSITION_CHECK_ITER)))
      return;
  setPos (newPos);
}

void Player::moveUp (double moveBy) {
  Vec3d newPos = m_pos + m_viewFrame.dirUp () * moveBy;
  for (int i = 0; i < POSITION_CHECK_ITER; ++i)
    if (!positionIsValid ((m_pos * i + newPos * (POSITION_CHECK_ITER - i)) / double (POSITION_CHECK_ITER)))
      return;
  setPos (newPos);
}

void Player::moveRight (double moveBy) {
  Vec3d newPos = m_pos + m_viewFrame.dirRight () * moveBy;
  for (int i = 0; i < POSITION_CHECK_ITER; ++i)
    if (!positionIsValid ((m_pos * i + newPos * (POSITION_CHECK_ITER - i)) / double (POSITION_CHECK_ITER)))
      return;
  setPos (newPos);
}


#ifdef CLIENT_APP  // TODO: delete

static inline Vec3i getCubeByPoint (Vec3d point, Vec3d direction) {
  return worldToCube (point + direction * 0.00001);
}

CubeWithFace Player::getHeadOnCube() {
//   return CubeWithFace (worldToCube (m_viewFrame.origin ()), Direction::X_PLUS);

  Vec3d currentPoint = m_viewFrame.origin ();
  Vec3d forwardVector = -m_viewFrame.dirForward ();  // TODO: ???
  Vec3i mapCenter = Vec3i::replicated (MAP_SIZE / 2);

  Vec3i cube = getCubeByPoint (currentPoint, forwardVector);
  Vec3i prevCube = cube;
  while  (  cubeIsValid (cube + mapCenter)
         && simpleWorldMap.get (cube + mapCenter).type == BT_AIR) {
    Vec3d parameter;
    Vec3d nearestInt;
    for (int i = 0; i < 3; ++i) {
      if (forwardVector[i] > 0)
        nearestInt[i] = floor (currentPoint[i]) + 1;
      else
      if (forwardVector[i] < 0)
        nearestInt[i] = ceil  (currentPoint[i]) - 1;
      else
        nearestInt[i] = 1;
      parameter[i] = (nearestInt[i] - currentPoint[i]) / forwardVector[i];
      assert (parameter[i] >= 0);
    }
    float t = xMax (xMin (parameter[0], parameter[1], parameter[2]), 1e-3);

    currentPoint += forwardVector * t;

    prevCube = cube;
    cube = getCubeByPoint (currentPoint, forwardVector);
  }
  return CubeWithFace (cube, getAdjacentFace (cube, prevCube));
}

#endif // CLIENT_APP


bool Player::positionIsValid (Vec3d pos) {
  return !BlockInfo::isSolid (simpleWorldMap.get (worldToCube (pos) + Vec3i::replicated (MAP_SIZE / 2)).type);
}
