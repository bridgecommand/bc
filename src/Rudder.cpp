#include "Rudder.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif 

Rudder::Rudder()
{
  Init(0,0,0,0,0,0,0,0,0,0,{0,0},0);
  mDelta = 0;
  mT << 0, 0, 0;
}

void Rudder::Init(double aHr, double aAr, double aXpR, double aAh, double aTr, double aXpH, double aEpsilon, double aKappa, double aLpR, double aLambdaR, std::vector<double> aGammaR, double aRrMax)
{
  mHr = aHr;
  mAr = aAr;
  mXpR = aXpR;
  mAh = aAh;
  mTr = aTr;
  mXpH = aXpH;
  mEpsilon = aEpsilon;
  mKappa = aKappa;
  mLpR = aLpR;
  mLambdaR = aLambdaR;
  mGammaR = aGammaR;
  mRrMax = aRrMax;
}

void Rudder::SetDelta(const double aDelta, const double aDt)
{
  double rrSet = 0;
  rrSet = (aDelta - mDelta) / aDt;

  if(abs(rrSet) > mRrMax) mDelta += ((rrSet/abs(rrSet)) * mRrMax * aDt);
  else mDelta = aDelta;
}

void Rudder::ComputeT(const Eigen::Vector3d& aMu, const double aRho, const sGeoParams& aGeo, const Propeller& aProp)
{
  double u = 0, beta = 0, rp = 0, lambdar = 0;
  double betap = 0, wp = 0, up = 0, jp = 0;
  double kt = 0, eta = 0, tmpur = 0, ur = 0;
  double betar = 0, gammar = 0, vr = 0, alphar = 0;
  double falpha = 0, Fn =0, xr = 0, xh = 0, Xr = 0, Yr = 0, Nr = 0;
  
  u = pow((pow(aMu[0], 2) + pow(aMu[1], 2)), 0.5);

  if(0 != aMu[0])
    beta = atan(-(aMu[1])/aMu[0]);
  else
    beta = 0;
  
  if(0 != aMu[0])
    rp = (aMu[2] * aGeo.lPP)/u;
  else
    u = 0;

  betap = beta - (aProp.getLongPos() * rp);
  
  wp = aProp.getWakeFraction() * exp(-4 * pow(betap, 2));
  up = aMu[0] * (1-wp);

  if(0 != aProp.getRevs() && 0 != aProp.getDiameter())
    jp = up / (aProp.getRevs() * aProp.getDiameter());
  else
    jp = 0;
  
  kt = aProp.getPolynomialCoef(0) + (aProp.getPolynomialCoef(1)*jp) + (aProp.getPolynomialCoef(2)*pow(jp, 2));

  if(0 != mHr)
    eta = aProp.getDiameter() / mHr;
  else
    eta = 0;

  if(0 != jp)
    tmpur = 1 + mKappa * (pow(1 + (8*kt/(M_PI*pow(jp, 2)) ), 0.5) - 1);
  else
    tmpur = 0;

  ur = mEpsilon * up * pow((eta * pow(tmpur, 2) + (1-eta)), 0.5);
  betar = beta - (mLpR * rp);

  if(betar < 0) gammar = mGammaR[0];
  else gammar = mGammaR[1];

  vr = u * gammar * betar;
  if(0 != ur)
    alphar = mDelta - atan(vr/ur);
  else
    alphar = 0;
  
  ur = pow((pow(ur, 2) + pow(vr, 2)), 0.5);

  if(0 != mLambdaR) lambdar = mLambdaR;
  else lambdar = pow(mHr, 2)/mAr;

  falpha = 6.13 * lambdar / (lambdar + 2.25);
  Fn = 0.5 * aRho * mAr * pow(ur, 2) * falpha * sin(alphar);
  xr = mXpR * aGeo.lPP;
  xh = mXpH * aGeo.lPP;

  Xr = -(1 - mTr) * Fn * sin(mDelta);
  Yr = -(1 + mAh) * Fn * cos(mDelta);
  Nr = -(xr + mAh * xh) * Fn * cos(mDelta);

  mT << Xr, Yr, Nr;
  //std::cout << "****Rudder mT :" << mT << std::endl; 
  return;
}

Eigen::Vector3d& Rudder::getT(void){return mT;}

double Rudder::getDelta(void){return mDelta;}
