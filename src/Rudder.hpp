#ifndef RUDDER_H
#define RUDDER_H

#include <Eigen/Dense>
#include <vector>
#include "ShipGlobalParams.hpp"
#include "Propeller.hpp"

class Rudder
{
public:
  
  Rudder();

  /*Process*/
  void Init(double mHr, double aAr, double aXpR, double aAh, double aTr, double aXpH, double aEpsilon, double aKappa, double aLpR, double aLambdaR, std::vector<double> mGammaR, double aRrMax, double aDeltaMax);
  void ComputeT(const Eigen::Vector3d& aMu, const double aRho, const sGeoParams& aGeo, const Propeller& aProp);

  /*Setter*/
  void SetDelta(double aDelta, const double aDt);

  /*Getter*/
  Eigen::Vector3d& getT(void);
  double getDelta(void) const;
  double getDeltaMax(void);
  
private:

  double mHr; //Span lenght
  double mAr; //Area of mobile part
  double mXpR; //Rudder longitudinal coordinate ratio  
  double mAh; //Rudder force increase factor
  double mTr; //Steering resistance deduction factor
  double mXpH; //Longitudinal coordinate of acting point of the additional lateral force component induced by steering
  double mEpsilon; //Ratio of wake fraction at propeller and rudder positions
  double mKappa; //Experimental constant for expressing uR
  double mLpR; //Effective longitudinal coordinate of rudder position in formula of bR ratio
  double mLambdaR; //Rudder aspect ratio
  std::vector<double> mGammaR; //Flow straightening coefficient
  double mRrMax; //Max rudder speed
  double mDeltaMax; //RUdder angle max
  
  Eigen::Vector3d mT; //Rudder force generated 
  double mDelta; //Rudder angle
};

#endif
