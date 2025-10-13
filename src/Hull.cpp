#include "Hull.hpp"
#include <cmath>

Hull::Hull(void)
{
  Init(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
  mT << 0, 0, 0;
}

void Hull::Init(double aXp0, double aXpVV, double aXpVR, double aXpRR, double aXpVVVV, double aYpV, double aYpR, double aYpVVV,
		double aYpVVR, double aYpVRR, double aYpRRR, double aNpV, double aNpR, double aNpVVV, double aNpVVR,
		double aNpVRR, double aNpRRR)
{
  mXp0 = aXp0;
  mXpVV = aXpVV;
  mXpVR = aXpVR; 
  mXpRR = aXpRR;
  mXpVVVV = aXpVVVV;
  mYpV = aYpV;
  mYpR = aYpR;
  mYpVVV = aYpVVV;
  mYpVVR = aYpVVR;
  mYpVRR = aYpVRR;
  mYpRRR = aYpRRR;
  mNpV = aNpV;
  mNpR = aNpR;
  mNpVVV = aNpVVV;
  mNpVVR = aNpVVR;
  mNpVRR = aNpVRR;
  mNpRRR = aNpRRR;
}
  

void Hull::ComputeT(const Eigen::Vector3d& aMu, const double aRho, const sGeoParams& aGeo)
{
  double u = 0, kf = 0, km = 0, vp = 0;
  double rp = 0, xph = 0, yph = 0, nph = 0;

  u = pow((pow(aMu[0], 2) + pow(aMu[1], 2)), 0.5);
  kf = 0.5 * aRho * aGeo.lPP * aGeo.d * pow(u , 2);
  km = 0.5 * aRho * pow(aGeo.lPP, 2) * aGeo.d * pow(u , 2);

  if(0 !=  u)
    {
      vp = aMu[1] / u;
      rp = aMu[2] * aGeo.lPP/u;
    }
  else
    {
      vp = 0;
      rp = 0;
    }

  xph = -mXp0 + (mXpVV * pow(vp, 2)) + (mXpVR * vp * rp) + (mXpRR * pow(rp, 2)) + (mXpVVVV * pow(vp, 4));
  
  yph = (mYpV * vp) + (mYpR * rp) + (mYpVVV * pow(vp, 3)) + (mYpVVR * pow(vp, 2) * rp) + (mYpVRR * pow(rp, 2) * vp) + (mYpRRR * pow(rp, 3));

  nph = (mNpV * vp) + (mNpR * rp) + (mNpVVV * pow(vp, 3)) + (mNpVVR * pow(vp, 2) * rp) + (mNpVRR * pow(rp, 2) * vp) + (mNpRRR * pow(rp, 3));

  mT << kf*xph, kf*yph, km*nph;

  //std::cout << "****Hull mT :" << mT << std::endl; 
  return;
}

Eigen::Vector3d& Hull::getT(void){return mT;}
