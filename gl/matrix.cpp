#include "matrix.h"
#include "string.h"
typedef unsigned int wint_t;
#include <math.h>


bool CMatrix::bInitialized_(false);
GLfixed CMatrix::fpSin_[360];
GLfixed CMatrix::fpCos_[360];


//---------------------------------------------------------------------------
CMatrix::CMatrix()
{
  if(bInitialized_ == false)
  {
    bInitialized_ = true;
    for(int i(0); i < 360; i++)
    {
      fpSin_[i] = m_fpfromf(sin(static_cast<float>(i) * M_PI / 180.0f));
      fpCos_[i] = m_fpfromf(cos(static_cast<float>(i) * M_PI / 180.0f));
    }
  }

  clear();
}

//---------------------------------------------------------------------------
CMatrix::~CMatrix()
{
}

//---------------------------------------------------------------------------
void
CMatrix::clear()
{
  memset(matrix, 0, sizeof(GLfixed) * 16);
}

//---------------------------------------------------------------------------
void
CMatrix::loadIdentity()
{
  clear();
  matrix[0][0] = m_fpfromi(1);
  matrix[1][1] = m_fpfromi(1);
  matrix[2][2] = m_fpfromi(1);
  matrix[3][3] = m_fpfromi(1);
}

//---------------------------------------------------------------------------
void
CMatrix::translate(GLfixed x, GLfixed y, GLfixed z)
{
  CMatrix m;
  m.loadIdentity();
  m.matrix[0][3] = x;
  m.matrix[1][3] = y;
  m.matrix[2][3] = z;
  *this *= m;
}

//---------------------------------------------------------------------------
void
CMatrix::translate(GLfixed * vec)
{
  translate(vec[0], vec[1], vec[2]);
}

//---------------------------------------------------------------------------
void
CMatrix::scale(GLfixed x, GLfixed y, GLfixed z)
{
  CMatrix m;
  m.loadIdentity();
  m.matrix[0][0] = x;
  m.matrix[1][1] = y;
  m.matrix[2][2] = z;
  *this *= m;
}

//---------------------------------------------------------------------------
void
CMatrix::scale(GLfixed * vec)
{
  scale(vec[0], vec[1], vec[2]);
}

//---------------------------------------------------------------------------
void
CMatrix::rotate(GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
  while(angle < 0)
    angle += m_fpfromi(360);
  GLfixed iSin = fpSin_[m_fptoi(angle) % 360];
  GLfixed iCos = fpCos_[m_fptoi(angle) % 360];

  long flags(((z != 0) << 2) | ((y != 0) << 1) | (x != 0));
  switch(flags)
  {
    case 0x00:
    {
      break;
    }
    case 0x01:
    {
      // X Rotation only
      CMatrix rot;
      rot.loadIdentity();
      rot.matrix[1][1] = iCos;
      rot.matrix[1][2] = -iSin;
      rot.matrix[2][1] = iSin;
      rot.matrix[2][2] = iCos;
      *this *= rot;
      break;
    }
    case 0x02:
    {
      // Y Rotation only
      CMatrix rot;
      rot.loadIdentity();
      rot.matrix[0][0] = iCos;
      rot.matrix[0][2] = iSin;
      rot.matrix[2][0] = -iSin;
      rot.matrix[2][2] = iCos;
      *this *= rot;
      break;
    }
    case 0x04:
    {
      // Z Rotation only
      CMatrix rot;
      rot.loadIdentity();
      rot.matrix[0][0] = iCos;
      rot.matrix[0][1] = -iSin;
      rot.matrix[1][0] = iSin;
      rot.matrix[1][1] = iCos;
      *this *= rot;
      break;
    }
    default:
    {
/*
      // Mixed Rotation
      CMatrix rot;
      rot.loadIdentity();
      x = m_to_s(x);
      y = m_to_s(y);
      z = m_to_s(z);
      rot.matrix[0][0] = s_to_m(s_fpmul(x, x) + s_fpmul(iCos, s_fpfromi(1) - s_fpmul(x, x)));
      rot.matrix[1][0] = s_to_m(s_fpmul(s_fpmul(x, y), (s_fpfromi(1) - iCos) - s_fpmul(z, iSin)));
      rot.matrix[2][0] = s_to_m(s_fpmul(s_fpmul(z, x), (s_fpfromi(1) - iCos) + s_fpmul(y, iSin)));
      rot.matrix[0][1] = s_to_m(s_fpmul(s_fpmul(x, y), (s_fpfromi(1) - iCos) + s_fpmul(z, iSin)));
      rot.matrix[1][1] = s_to_m(s_fpmul(y, y) + s_fpmul(iCos, s_fpfromi(1) - s_fpmul(y, y)));
      rot.matrix[2][1] = s_to_m(s_fpmul(s_fpmul(y, z), (s_fpfromi(1) - iCos) - s_fpmul(x, iSin)));
      rot.matrix[0][2] = s_to_m(s_fpmul(s_fpmul(y, z), (s_fpfromi(1) - iCos) - s_fpmul(x, iSin)));
      rot.matrix[1][2] = s_to_m(s_fpmul(s_fpmul(y, z), (s_fpfromi(1) - iCos) + s_fpmul(x, iSin)));
      rot.matrix[2][2] = s_to_m(s_fpmul(z, z) + s_fpmul(iCos, s_fpfromi(1) - s_fpmul(z, z)));
      rot.matrix[3][3] = m_fpfromi(1);
      *this *= rot;
*/
    }
  };
}

//---------------------------------------------------------------------------
void
CMatrix::rotate(GLfixed * angles)
{
  rotate(angles[0], angles[1], angles[2], angles[3]);
}

//---------------------------------------------------------------------------
void
CMatrix::transform(const GLfixed * from, GLfixed * to)
{
  GLfixed x(from[0]);
  GLfixed y(from[1]);
  GLfixed z(from[2]);
  GLfixed w(from[3]);
  to[0] = m_fpmul(matrix[0][0], x) + m_fpmul(matrix[0][1], y) + m_fpmul(matrix[0][2], z) + m_fpmul(matrix[0][3], w);
  to[1] = m_fpmul(matrix[1][0], x) + m_fpmul(matrix[1][1], y) + m_fpmul(matrix[1][2], z) + m_fpmul(matrix[1][3], w);
  to[2] = m_fpmul(matrix[2][0], x) + m_fpmul(matrix[2][1], y) + m_fpmul(matrix[2][2], z) + m_fpmul(matrix[2][3], w);
  to[3] = m_fpmul(matrix[3][0], x) + m_fpmul(matrix[3][1], y) + m_fpmul(matrix[3][2], z) + m_fpmul(matrix[3][3], w);
}

//---------------------------------------------------------------------------
CMatrix
CMatrix::operator*(const CMatrix & m)
{
  CMatrix mReturn(*this);

  mReturn *= *this;

  return(mReturn);
}

//---------------------------------------------------------------------------
// 64 x operator*
// 48 x operator+
CMatrix &
CMatrix::operator*=(const CMatrix & m)
{
  GLfixed mtemp[4][4];

  for(int iRow(0); iRow < 4; iRow++)
  {
    for(int iCol(0); iCol < 4; iCol++)
    {
      mtemp[iRow][iCol] = m_fpmul(matrix[iRow][0], m.matrix[0][iCol]) +
                          m_fpmul(matrix[iRow][1], m.matrix[1][iCol]) +
                          m_fpmul(matrix[iRow][2], m.matrix[2][iCol]) +
                          m_fpmul(matrix[iRow][3], m.matrix[3][iCol]);
    }
  }

  memcpy(matrix, mtemp, sizeof(GLfixed) * 16);

  return(*this);
}
