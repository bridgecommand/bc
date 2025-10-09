#include "Propeller.hpp"
#include <cmath>

Propeller::Propeller(void)
{
  Init(0,0,0,0,0,0,0);
  mT << 0, 0, 0;
}

void Propeller::Init(double aDiam, double aTfactor, double aXp, double aW0fraction, double aK0, double aK1, double aK2)
{
  mDiam = aDiam;
  mTfactor = aTfactor;
  mXp = aXp;
  mW0fraction = aW0fraction;
  mK0 = aK0; 
  mK1 = aK1;
  mK2 = aK2;
}

void Propeller::SetRevs(const double aNrps)
{
  mNrps = aNrps;
}

void Propeller::ComputeT(const Eigen::Vector3d& aMu, const double aRho, const sGeoParams& aGeo)
{
  double u = 0, beta = 0, rp = 0;
  double betap = 0, wp = 0, up = 0, jp = 0;
  double kt = 0, tp = 0, xp = 0;

  u = pow((pow(aMu[0], 2) + pow(aMu[1], 2)), 0.5);
  beta = atan(-(aMu[1])/aMu[0]);
  rp = (aMu[2] * aGeo.lPP)/u;
  betap = beta - (mXp * rp);

  wp = mW0fraction * exp(-4 * pow(betap, 2));
  up = aMu[0] * (1-wp);
  jp = up / (mNrps * mDiam);
  kt = mK0 + (mK1*jp) + (mK2*pow(jp, 2));
  tp = aRho * pow(mNrps, 2) * pow(mDiam, 4) * kt;
  xp = (1-mTfactor) * tp;

  mT << xp, 0, 0;

  std::cout << "***Propeller mT :" << mT << std::endl;
  return;
}

Eigen::Vector3d& Propeller::getPropellerT(void){return mT;}
