#include <cassert>

#ifdef CLIENT_APP
  #include "client/client_world.hpp"
#else // !CLIENT_APP
  #include "server/server_world.hpp"
#endif // !CLIENT_APP

#include "common/player.hpp"



Player::Player() {
  setPos (Vec3d::zero());
  m_blockInHand = BT_BRICKS;
  m_flying = true;
}

Player::~Player() { }


  // TODO: delete that and rewrite normally
static const int POSITION_CHECK_ITER = 10;

void Player::setPos (Vec3d newPos) {
  m_pos = newPos;
  m_viewFrame.setOrigin (m_pos + Vec3d (0., 0., EYE_HEIGHT));
}

void Player::moveForward (double moveBy) {
  if (m_flying)
    doMove (m_viewFrame.dirForward(), moveBy);
  else
    doMove (L2::normalize (Vec3d (m_viewFrame.dirForward().x(), m_viewFrame.dirForward().y(), 0.)), moveBy);
}

void Player::moveUp (double moveBy) {
  if (m_flying)
    doMove (m_viewFrame.dirUp(), moveBy);
}

void Player::moveRight (double moveBy) {
  if (m_flying)
    doMove (m_viewFrame.dirRight(), moveBy);
  else
    doMove (L2::normalize (Vec3d (m_viewFrame.dirRight().x(), m_viewFrame.dirRight().y(), 0.)), moveBy);
}


void Player::setFlying (bool flyingState) {
  m_flying = flyingState;
//   if (!m_flying)
//     setPos (simpleWorldMap.getGroundBeneathPos (m_pos));
}


#ifdef CLIENT_APP  // TODO: delete

static inline Vec3i getCubeByPoint (Vec3d point, Vec3d direction) {
  return worldToCube (point + direction * 0.00001);
}

CubeWithFace Player::getHeadOnCube() const {
//   return CubeWithFace (worldToCube (m_viewFrame.origin()), Direction::X_PLUS);

  Vec3d currentPoint = m_viewFrame.origin();
  Vec3d forwardVector = m_viewFrame.dirForward();  // TODO: ???
  Vec3i mapCenter = Vec3i::replicated (MAP_SIZE / 2);

  Vec3i cube = getCubeByPoint (currentPoint, forwardVector);
  Vec3i prevCube = cube;
  while  (   cubeIsValid (cube + mapCenter)
          && !BlockInfo::isSolid (simpleWorldMap.get (cube + mapCenter).type)) {
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


void Player::processPlayer (double timeDelta) {
  if (m_flying)
    return;

}


static inline bool blockIsFree (Vec3d pos) {
  return !BlockInfo::isSolid (simpleWorldMap.get (worldToCube (pos) + Vec3i::replicated (MAP_SIZE / 2)).type);
}

bool Player::positionIsValid (Vec3d pos) {
  return blockIsFree (pos) && blockIsFree (pos + Vec3d (0., 0., BODY_HEIGHT));
}

void Player::doMove (Vec3d direction, double moveBy) {
  Vec3d newPos = m_pos + direction * moveBy;
  for (int i = 0; i < POSITION_CHECK_ITER; ++i)
    if (!positionIsValid ((m_pos * i + newPos * (POSITION_CHECK_ITER - i)) / double (POSITION_CHECK_ITER)))
      return;
  setPos (newPos);
}
