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
  m_flying = false;
  m_upVelocity = 0.;
}

Player::~Player() { }


void Player::setPos (Vec3d newPos) {
  m_pos = newPos;
  m_viewFrame.setOrigin (m_pos + Vec3d (0., 0., EYE_HEIGHT));
}

void Player::moveForward (double moveBy) {
  if (m_flying)
    doMove (m_viewFrame.dirForward(), moveBy * FLYING_SPEED_COEFF);
  else
    doMove (L2::normalize (Vec3d (m_viewFrame.dirForward().x(), m_viewFrame.dirForward().y(), 0.)), inAir() ? moveBy * AIR_SPEED_COEFF : moveBy);
}

void Player::moveUp (double moveBy) {
  if (m_flying)
    doMove (m_viewFrame.dirUp(), moveBy * FLYING_SPEED_COEFF);
}

void Player::moveRight (double moveBy) {
  if (m_flying)
    doMove (m_viewFrame.dirRight(), moveBy * FLYING_SPEED_COEFF);
  else
    doMove (L2::normalize (Vec3d (m_viewFrame.dirRight().x(), m_viewFrame.dirRight().y(), 0.)), inAir() ? moveBy * AIR_SPEED_COEFF : moveBy);
}

void Player::jump () {
  if (!m_flying && !inAir())
    m_upVelocity += JUMPING_ACCELERATION;
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
          && !BlockInfo::isSolid (simpleWorldMap.get (cube + mapCenter))) {
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
  if (!m_flying) {
    m_upVelocity -= GRAVITY * timeDelta;
    if (!tryToMove (Vec3d::e3 (), m_upVelocity * timeDelta))
      m_upVelocity = 0.;
  }
}


bool Player::inAir() {
  return !BlockInfo::isSolid (simpleWorldMap.get (worldToCube (m_pos - Vec3d (0., 0., MAX_SOARING_HEIGHT)) + Vec3i::replicated (MAP_SIZE / 2)));
}

bool Player::tryToMove (Vec3d direction, double moveBy) {
  while (xAbs (moveBy) > MAX_SOARING_HEIGHT / 4.) {
    double step = xMin (MAX_MOVEMENT_DELTA, moveBy);
    Vec3d newPos = m_pos + direction * step;
    if (!positionIsValid (newPos)) {
      step /= 2;
      while (xAbs (step) > MAX_SOARING_HEIGHT / 2.) {
        newPos = m_pos + direction * step;
        if (positionIsValid (newPos))
          setPos (newPos);
        step /= 2;
      }
      return false;
    }
    setPos (m_pos + direction * step);
    moveBy -= step;
  }
  return true;
}

void Player::doMove (Vec3d direction, double moveBy) {
//   tryToMove (direction, moveBy);
  for (int coord = 0; coord < 3; ++coord)
    tryToMove (Vec3d::e_i (coord), direction[coord] * moveBy);
}


static inline bool blockIsFree (Vec3d pos) {
  return !BlockInfo::isSolid (simpleWorldMap.get (worldToCube (pos) + Vec3i::replicated (MAP_SIZE / 2)));
}

bool Player::positionIsValid (Vec3d pos) {
  double bodyWidth = BODY_WIDTH;
  return    blockIsFree (pos + Vec3d ( bodyWidth / 2.,  bodyWidth / 2., 0)) && blockIsFree (pos + Vec3d ( bodyWidth / 2.,  bodyWidth / 2., 0) + Vec3d (0., 0., BODY_HEIGHT))
         && blockIsFree (pos + Vec3d ( bodyWidth / 2., -bodyWidth / 2., 0)) && blockIsFree (pos + Vec3d ( bodyWidth / 2., -bodyWidth / 2., 0) + Vec3d (0., 0., BODY_HEIGHT))
         && blockIsFree (pos + Vec3d (-bodyWidth / 2.,  bodyWidth / 2., 0)) && blockIsFree (pos + Vec3d (-bodyWidth / 2.,  bodyWidth / 2., 0) + Vec3d (0., 0., BODY_HEIGHT))
         && blockIsFree (pos + Vec3d (-bodyWidth / 2., -bodyWidth / 2., 0)) && blockIsFree (pos + Vec3d (-bodyWidth / 2., -bodyWidth / 2., 0) + Vec3d (0., 0., BODY_HEIGHT));
}
