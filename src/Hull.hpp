#ifndef HULL_HPP
#define HULL_HPP

#include <Eigen/Dense>
#include <vector>
#include "ShipGlobalParams.hpp"

class Hull
{
public:
  
  Hull();

  /*Process*/
  void Init(double aXp0, double aXpVV, double aXpVR, double aXpRR, double aXpVVVV, double aYpV, double aYpR, double aYpVVV,
		double aYpVVR, double aYpVRR, double aYpRRR, double aNpV, double aNpR, double aNpVVV, double aNpVVR,
		double aNpVRR, double aNpRRR);
  void ComputeT(const Eigen::Vector3d& aMu, const double aRho, const sGeoParams& aGeo);

  /*Getter*/
  Eigen::Vector3d& getT(void);
  void PrintParams(void);
  
private: 

  double mXp0;
  double mXpVV;
  double mXpVR;
  double mXpRR;
  double mXpVVVV;
  double mYpV;
  double mYpR;
  double mYpVVV;
  double mYpVVR;
  double mYpVRR;
  double mYpRRR;
  double mNpV;
  double mNpR;
  double mNpVVV;
  double mNpVVR;
  double mNpVRR;
  double mNpRRR;
  
  Eigen::Vector3d mT;
};

#endif
