#ifndef PLAYER_HPP
#define PLAYER_HPP


#include "linear_algebra.hpp"


// TODO: delete
#include "client/GLTools/include/math3d.h"


// TODO: orientation --- ?
class ViewFrame {
public:
  // Default position and orientation. At the origin, looking
  // down the positive Z axis (right handed coordinate system).    // TODO: What???
  ViewFrame () :
    m_origin      (0., 0., 0.),
    m_dirForward  (0., 0.,-1.), // Forward is -Z (default OpenGL)
    m_dirUp       (0., 1., 0.)  // Up is +Y
  {
  }

  void setOrigin (Vec3d newOrigin)                          { m_origin = newOrigin; }
  void setOrigin (double x, double y, double z)             { m_origin.setCoordinates (x, y, z); }
  const Vec3d& origin () const                              { return m_origin; }

  void setDirForward (Vec3d newDirForward)                  { m_dirForward = newDirForward; }
  void setDirForward (double x, double y, double z)         { m_dirForward.setCoordinates (x, y, z); }
  const Vec3d& dirForward () const                          { return m_dirForward; }

  void setDirUp (Vec3d newDirUp)                            { m_dirUp = newDirUp; }
  void setDirUp (double x, double y, double z)              { m_dirUp.setCoordinates (x, y, z); }
  const Vec3d& dirUp () const                               { return m_dirUp; }


//   void TranslateWorld (float x, float y, float z) {
//     vOrigin[0] += x;
//     vOrigin[1] += y;
//     vOrigin[2] += z;
//   }
//
//   void TranslateLocal (float x, float y, float z) {
//     MoveForward (z);
//     MoveUp (y);
//     MoveRight (x);
//   }


  void moveForward (double moveBy)                          { m_origin += m_dirForward * moveBy; }
  void moveUp (double moveBy)                               { m_origin += m_dirUp * moveBy; }
  void moveRight (double moveBy)                            { m_origin += crossProduct (m_dirUp, m_dirForward) * moveBy; }


  // TODO: rewrite
  void getCameraMatrix (M3DMatrix44f result /*, bool rotationOnly = false*/) {
    Vec3d z = -m_dirForward;
    Vec3d x = crossProduct (m_dirUp, z);

    // Matrix has no translation information and is
    // transposed.... (rows instead of columns)
#define M(row,col)  result[col*4+row]
    M (0, 0) = x[0];
    M (0, 1) = x[1];
    M (0, 2) = x[2];
    M (0, 3) = 0.0;
    M (1, 0) = m_dirUp[0];
    M (1, 1) = m_dirUp[1];
    M (1, 2) = m_dirUp[2];
    M (1, 3) = 0.0;
    M (2, 0) = z[0];
    M (2, 1) = z[1];
    M (2, 2) = z[2];
    M (2, 3) = 0.0;
    M (3, 0) = 0.0;
    M (3, 1) = 0.0;
    M (3, 2) = 0.0;
    M (3, 3) = 1.0;
#undef M

//     if (rotationOnly)
//       return;
//
//     // Apply translation too
//     M3DMatrix44f trans, M;
//     m3dTranslationMatrix44 (trans, -vOrigin[0], -vOrigin[1], -vOrigin[2]);
//
//     m3dMatrixMultiply44 (M, m, trans);
//
//     // Copy result back into m
//     memcpy (m, M, sizeof (float) *16);
  }


// TODO: can't this fucntion be optimized?
  void RotateLocalX (double angle) {
    M3DMatrix33d rotMat;
    Vec3d localX = crossProduct (m_dirUp, m_dirForward);
    Vec3d rotVec;

    // Make a Rotation Matrix
    m3dRotationMatrix33 (rotMat, angle, localX[0], localX[1], localX[2]);

    // Rotate Y, and Z
    m3dRotateVector (rotVec.coords, m_dirUp.coords, rotMat);
    m3dCopyVector3 (m_dirUp.coords, rotVec.coords);

    m3dRotateVector (rotVec.coords, m_dirForward.coords, rotMat);
    m3dCopyVector3 (m_dirForward.coords, rotVec.coords);
  }

//   // Rotate around local Y
//   void RotateLocalY (float fAngle) {
//     M3DMatrix44f rotMat;
//
//     // Just Rotate around the up vector
//     // Create a rotation matrix around my Up (Y) vector
//     m3dRotationMatrix44 (rotMat, fAngle,
//                          vUp[0], vUp[1], vUp[2]);
//
//     M3DVector3f newVect;
//
//     // Rotate forward pointing vector (inlined 3x3 transform)
//     newVect[0] = rotMat[0] * vForward[0] + rotMat[4] * vForward[1] + rotMat[8] *  vForward[2];
//     newVect[1] = rotMat[1] * vForward[0] + rotMat[5] * vForward[1] + rotMat[9] *  vForward[2];
//     newVect[2] = rotMat[2] * vForward[0] + rotMat[6] * vForward[1] + rotMat[10] * vForward[2];
//     m3dCopyVector3 (vForward, newVect);
//   }
//
//   // Rotate around local Z
//   void RotateLocalZ (float fAngle) {
//     M3DMatrix44f rotMat;
//
//     // Only the up vector needs to be rotated
//     m3dRotationMatrix44 (rotMat, fAngle,
//                          vForward[0], vForward[1], vForward[2]);
//
//     M3DVector3f newVect;
//     newVect[0] = rotMat[0] * vUp[0] + rotMat[4] * vUp[1] + rotMat[8] *  vUp[2];
//     newVect[1] = rotMat[1] * vUp[0] + rotMat[5] * vUp[1] + rotMat[9] *  vUp[2];
//     newVect[2] = rotMat[2] * vUp[0] + rotMat[6] * vUp[1] + rotMat[10] * vUp[2];
//     m3dCopyVector3 (vUp, newVect);
//   }


//   // Reset axes to make sure they are orthonormal. This should be called on occasion
//   // if the matrix is long-lived and frequently transformed.
//   void Normalize (void) {
//     M3DVector3f vCross;
//
//     m3dCrossProduct3 (vCross, vUp, vForward);
//     m3dCrossProduct3 (vForward, vCross, vUp);
//
//     m3dNormalizeVector3 (vUp);
//     m3dNormalizeVector3 (vForward);
//   }


//   void RotateWorld (float fAngle, float x, float y, float z) {
//     M3DMatrix44f rotMat;
//
//     // Create the Rotation matrix
//     m3dRotationMatrix44 (rotMat, fAngle, x, y, z);
//
//     M3DVector3f newVect;
//
//     // Transform the up axis (inlined 3x3 rotation)
//     newVect[0] = rotMat[0] * vUp[0] + rotMat[4] * vUp[1] + rotMat[8] *  vUp[2];
//     newVect[1] = rotMat[1] * vUp[0] + rotMat[5] * vUp[1] + rotMat[9] *  vUp[2];
//     newVect[2] = rotMat[2] * vUp[0] + rotMat[6] * vUp[1] + rotMat[10] * vUp[2];
//     m3dCopyVector3 (vUp, newVect);
//
//     // Transform the forward axis
//     newVect[0] = rotMat[0] * vForward[0] + rotMat[4] * vForward[1] + rotMat[8] *  vForward[2];
//     newVect[1] = rotMat[1] * vForward[0] + rotMat[5] * vForward[1] + rotMat[9] *  vForward[2];
//     newVect[2] = rotMat[2] * vForward[0] + rotMat[6] * vForward[1] + rotMat[10] * vForward[2];
//     m3dCopyVector3 (vForward, newVect);
//   }
//
//   void RotateLocal (float fAngle, float x, float y, float z) {
//     M3DVector3f vWorldVect;
//     M3DVector3f vLocalVect;
//     m3dLoadVector3 (vLocalVect, x, y, z);
//
//     LocalToWorld (vLocalVect, vWorldVect, true);
//     RotateWorld (fAngle, vWorldVect[0], vWorldVect[1], vWorldVect[2]);
//   }


//   // Convert Coordinate Systems
//   // This is pretty much, do the transformation represented by the rotation
//   // and position on the point
//   // Is it better to stick to the convention that the destination always comes
//   // first, or use the conventions that "sounds" like the function...
//   void LocalToWorld (const M3DVector3f vLocal, M3DVector3f vWorld, bool bRotOnly = false) {
//     // Create the rotation matrix based on the vectors
//     M3DMatrix44f rotMat;
//
//     GetMatrix (rotMat, true);
//
//     // Do the rotation (inline it, and remove 4th column...)
//     vWorld[0] = rotMat[0] * vLocal[0] + rotMat[4] * vLocal[1] + rotMat[8] *  vLocal[2];
//     vWorld[1] = rotMat[1] * vLocal[0] + rotMat[5] * vLocal[1] + rotMat[9] *  vLocal[2];
//     vWorld[2] = rotMat[2] * vLocal[0] + rotMat[6] * vLocal[1] + rotMat[10] * vLocal[2];
//
//     // Translate the point
//     if (!bRotOnly) {
//       vWorld[0] += vOrigin[0];
//       vWorld[1] += vOrigin[1];
//       vWorld[2] += vOrigin[2];
//     }
//   }
//
//   // Change world coordinates into "local" coordinates
//   void WorldToLocal (const M3DVector3f vWorld, M3DVector3f vLocal) {
//     ////////////////////////////////////////////////
//     // Translate the origin
//     M3DVector3f vNewWorld;
//     vNewWorld[0] = vWorld[0] - vOrigin[0];
//     vNewWorld[1] = vWorld[1] - vOrigin[1];
//     vNewWorld[2] = vWorld[2] - vOrigin[2];
//
//     // Create the rotation matrix based on the vectors
//     M3DMatrix44f rotMat;
//     M3DMatrix44f invMat;
//     GetMatrix (rotMat, true);
//
//     // Do the rotation based on inverted matrix
//     m3dInvertMatrix44 (invMat, rotMat);
//
//     vLocal[0] = invMat[0] * vNewWorld[0] + invMat[4] * vNewWorld[1] + invMat[8] *  vNewWorld[2];
//     vLocal[1] = invMat[1] * vNewWorld[0] + invMat[5] * vNewWorld[1] + invMat[9] *  vNewWorld[2];
//     vLocal[2] = invMat[2] * vNewWorld[0] + invMat[6] * vNewWorld[1] + invMat[10] * vNewWorld[2];
//   }


protected:
  Vec3d m_origin;
  Vec3d m_dirForward;
  Vec3d m_dirUp;
};



// TODO: instead of inheriting ViewFrame, create a protected member ``ViewFrame viewFrame''
class Player : public ViewFrame {
public:
//   Vec3d pos () const                    { return m_pos; }
//   void setPos (Vec3d newPos)            { m_pos = newPos; }

protected:
//   Vec3d m_pos;
};


#endif
