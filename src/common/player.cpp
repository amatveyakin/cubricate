#include <cassert>

#include "common/world_block.hpp"
#include "common/player.hpp"

#ifdef CLIENT_APP
  #include "client/client_world.hpp"
#else // !CLIENT_APP
  #include "server/server_world.hpp"
#endif // !CLIENT_APP



const double Player::BODY_HEIGHT = 1.5;
const double Player::EYE_HEIGHT  = 1.4;
const double Player::BODY_WIDTH  = 0.7;
const double Player::JUMPING_ACCELERATION = 10.;
const double Player::FLYING_SPEED_COEFF = 2.;
const double Player::AIR_SPEED_COEFF = 0.3;

const double Player::MAX_SOARING_HEIGHT = 1e-3;
const double Player::MAX_MOVEMENT_DELTA = 1e-1;

const int    Player::N_VERTICAL_SEGMENTS = int (BODY_HEIGHT + 1.);


Player::Player() {
  setPos (Vec3d::zero());
  m_blockInHand = BT_TEST_LIGHT;
  m_flying = false;
  m_upVelocity = 0.;
  m_lastSelectedCube = Vec3i (-1, -1, -1);
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
    doMove (Vec3d (m_viewFrame.dirForward().x(), m_viewFrame.dirForward().y(), 0.), inAir() ? moveBy * AIR_SPEED_COEFF : moveBy);
}

void Player::moveUp (double moveBy) {
  if (m_flying)
    doMove (m_viewFrame.dirUp(), moveBy * FLYING_SPEED_COEFF);
}

void Player::moveRight (double moveBy) {
  if (m_flying)
    doMove (m_viewFrame.dirRight(), moveBy * FLYING_SPEED_COEFF);
  else
    doMove (Vec3d (m_viewFrame.dirRight().x(), m_viewFrame.dirRight().y(), 0.), inAir() ? moveBy * AIR_SPEED_COEFF : moveBy);
}

void Player::jump () {
  if (!m_flying && !inAir())
    m_upVelocity = xMax (m_upVelocity, JUMPING_ACCELERATION);
}


void Player::setFlying (bool flyingState) {
  m_flying = flyingState;
}


#ifdef CLIENT_APP  // TODO: delete


CubeWithFace Player::getHeadOnCube() const {
  // zapilit normalniy algoritm Brezenhama, bleyat
  Vec3d currentPoint = m_viewFrame.origin();
  Vec3d forwardVector = m_viewFrame.dirForward();

  Vec3i cube = getCubeByPoint (currentPoint, forwardVector);
  Vec3i prevCube = cube;
  while  (   cubeIsValid (cube)
          && !BlockInfo::isFirm (simpleWorldMap.get (cube))) {
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


bool Player::intersectsCube (Vec3i cube) const {
  for (int i = 0; i <= N_VERTICAL_SEGMENTS; ++i) {
    if (   worldToCube (m_pos + Vec3d ( BODY_WIDTH / 2.,  BODY_WIDTH / 2., BODY_HEIGHT * i / double (N_VERTICAL_SEGMENTS))) == cube
        || worldToCube (m_pos + Vec3d ( BODY_WIDTH / 2., -BODY_WIDTH / 2., BODY_HEIGHT * i / double (N_VERTICAL_SEGMENTS))) == cube
        || worldToCube (m_pos + Vec3d (-BODY_WIDTH / 2.,  BODY_WIDTH / 2., BODY_HEIGHT * i / double (N_VERTICAL_SEGMENTS))) == cube
        || worldToCube (m_pos + Vec3d (-BODY_WIDTH / 2., -BODY_WIDTH / 2., BODY_HEIGHT * i / double (N_VERTICAL_SEGMENTS))) == cube)
      return true;
  }
  return false;
}


void Player::processPlayer (double timeDelta) {
  if (!m_flying) {
    m_upVelocity -= GRAVITY * timeDelta;
    if (!tryToMove (Vec3d::e3 (), m_upVelocity * timeDelta))
      m_upVelocity = 0.;
  }
  updateCubeSelection();  // TODO: is it good to call it here?
}


bool Player::inAir() {
  return positionIsValid (m_pos - Vec3d (0., 0., MAX_SOARING_HEIGHT));
}

bool Player::tryToMove (Vec3d direction, double moveBy) {
  // Unstucking
//   while (!positionIsValid (m_pos))
//     m_pos += Vec3d::e3() * MAX_MOVEMENT_DELTA;

  direction = L2::normalize (direction);

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
//   updateCubeSelection();
}


void Player::updateCubeSelection() {
  // Deselecting previous cube
  if (cubeIsValid (m_lastSelectedCube)) {
    WorldBlock headOnBlock = simpleWorldMap.get (m_lastSelectedCube);
    headOnBlock.parameters = 0;
    simpleWorldMap.set (m_lastSelectedCube, headOnBlock);
  }

  // Selecting current cube
  Vec3i selectedCube = getHeadOnCube().cube;
  if (cubeIsValid (selectedCube)) {
    WorldBlock headOnBlock = simpleWorldMap.get (selectedCube);
    headOnBlock.parameters = 1;
    simpleWorldMap.set (selectedCube, headOnBlock);
    m_lastSelectedCube = selectedCube;
  }
  else
    m_lastSelectedCube = INVALID_CUBE;
}


static inline bool blockIsFree (Vec3d position) {
  return !BlockInfo::isFirm (simpleWorldMap.get (worldToCube (position)));
}

bool Player::positionIsValid (Vec3d position) {
  for (int i = 0; i <= N_VERTICAL_SEGMENTS; ++i) {
    if (   !blockIsFree (position + Vec3d ( BODY_WIDTH / 2.,  BODY_WIDTH / 2., BODY_HEIGHT * i / double (N_VERTICAL_SEGMENTS)))
        || !blockIsFree (position + Vec3d ( BODY_WIDTH / 2., -BODY_WIDTH / 2., BODY_HEIGHT * i / double (N_VERTICAL_SEGMENTS)))
        || !blockIsFree (position + Vec3d (-BODY_WIDTH / 2.,  BODY_WIDTH / 2., BODY_HEIGHT * i / double (N_VERTICAL_SEGMENTS)))
        || !blockIsFree (position + Vec3d (-BODY_WIDTH / 2., -BODY_WIDTH / 2., BODY_HEIGHT * i / double (N_VERTICAL_SEGMENTS))))
      return false;
  }
  return true;
}
