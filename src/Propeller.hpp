#ifndef PROPELLER_HPP
#define PROPELLER_HPP

#include <Eigen/Dense>
#include <vector>
#include "ShipGlobalParams.hpp"

class Propeller
{
public:

  Propeller();

  /*Process*/
  void Init(double aDiam, double aTfactor, double aXp, double aW0fraction, double aK0, double aK1, double aK2);
  void ComputeT(const Eigen::Vector3d& aMu, double aRho, const sGeoParams& aGeo);
  void SetRevs(const double aNrps);
  
  /*Getter*/
  Eigen::Vector3d& getPropellerT(void);

private:

  double mDiam; //Propeller diameter
  double mTfactor; //Thrust deduction factor
  double mXp; //longitudinal position of the propeller
  double mW0fraction; //Nominal wake fraction
  double mK0; //polynomial coefficients 
  double mK1; //Kt(J) = k0 + k1*J + k2*J²
  double mK2;

  double mNrps; //Propeller revolutions per second
  Eigen::Vector3d mT; //Propeller Force generated
};

#endif
