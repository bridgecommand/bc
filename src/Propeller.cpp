#include "Propeller.hpp"
#include <cmath>

Propeller::Propeller(void)
{
  Init(0,0,0,0,0,0,0,"right",0);
  mT << 0, 0, 0;
  mNrps = 0;
  mCurrentRotDir = "stop";
}

void Propeller::Init(double aDiam, double aTfactor, double aXp, double aW0fraction, double aK0, double aK1, double aK2, std::string aForwardRotDir, double aBackwardsEff)
{
  mDiam = aDiam;
  mTfactor = aTfactor;
  mXp = aXp;
  mW0fraction = aW0fraction;
  mK0 = aK0; 
  mK1 = aK1;
  mK2 = aK2;
  mForwardRotDir = aForwardRotDir;
  mBackwardsEff = aBackwardsEff;
}

void Propeller::SetRevs(const double aNrps)
{
  if(aNrps < 0)
    {
      if(0 == ChangeRotDir("backwards"))
	mNrps = abs(aNrps);
      else
	mNrps = 0;
    }
  else if(aNrps > 0)
    {
      if(0 == ChangeRotDir("forward"))
	mNrps = aNrps;
      else
	mNrps = 0;
    }
  else
    {
      ChangeRotDir("stop");
      mNrps = 0;
    }
}

int Propeller::ChangeRotDir(std::string aDir)
{
  int err = 0;

      if("forward" == aDir)
	mCurrentRotDir = mForwardRotDir;
      else if("backwards" == aDir)
	{
	  if("right" == mForwardRotDir)
	    mCurrentRotDir = "left";
	  else if("left" == mForwardRotDir)
	    mCurrentRotDir = "right";
	  else
	    mCurrentRotDir = "stop";
	}
      else
	mCurrentRotDir = "stop";
      
  return err;
}

void Propeller::ComputeT(const Eigen::Vector3d& aMu, const double aRho, const sGeoParams& aGeo)
{
  double u = 0, beta = 0, rp = 0;
  double betap = 0, wp = 0, up = 0, jp = 0;
  double kt = 0, tp = 0, xp = 0;

  u = pow((pow(aMu[0], 2) + pow(aMu[1], 2)), 0.5);

  if(0 != aMu[0] && 0 != u)
    {
      beta = atan(-(aMu[1])/aMu[0]);
      rp = (aMu[2] * aGeo.lPP)/u;
    }
  else
    {
      rp = 0;
      beta = 0;
    }  
  
  betap = beta - (mXp * rp);

  wp = mW0fraction * exp(-4 * pow(betap, 2));
  up = aMu[0] * (1-wp);

  if(0 != mNrps)
    jp = up / (mNrps * mDiam);
  else
    jp = 0;
  
  kt = mK0 + (mK1*jp) + (mK2*pow(jp, 2));
  tp = aRho * pow(mNrps, 2) * pow(mDiam, 4) * kt;
  xp = (1-mTfactor) * tp;

  //Add a efficiency factor for backwards
  if(mForwardRotDir != mCurrentRotDir)
    xp = xp*(-mBackwardsEff);
    
  mT << xp, 0, 0;

  //std::cout << "***Propeller mT :" << mT << std::endl;
  return;
}

Eigen::Vector3d Propeller::getT(void) const {return mT;}
double Propeller::getDiameter(void) const {return mDiam;}
double Propeller::getWakeFraction(void) const {return mW0fraction;}
double Propeller::getLongPos(void) const {return mXp;}
double Propeller::getRevs(void) const {return mNrps;}
std::string Propeller::getForwardRotDir(void) const {return mForwardRotDir;}
std::string Propeller::getCurrentRotDir(void) const {return mCurrentRotDir;}

double Propeller::getPolynomialCoef(unsigned char aCoefNumber) const
{
  if(0 == aCoefNumber) return mK0;
  else if(1 == aCoefNumber) return mK1;
  else if(2 == aCoefNumber) return mK2;
  else return -1;
}
