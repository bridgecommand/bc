#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <iostream>
#include <Eigen/Dense>
#include "ShipGlobalParams.hpp"
#include "Ship.hpp"
#include "Time.hpp"

#define VECTOR_SIZE_DIFF_EQ (6)

class Solver
{
public:

  /*Constructor*/
  Solver();
  Solver(Ship* aShip);
  
  /*Process*/
  int Init(Ship* aShip);
  void SolveRk4(sTime& aTime, Eigen::Vector3d aEta, Eigen::Vector3d aMu);

  /*Getter*/
  Eigen::Vector3d getEta(void) const;
  Eigen::Vector3d getMu(void) const;
  
  /*Setter*/
  void SetDeltaT(double aDt);
  
private:

  Eigen::VectorXd DiffEq(const Eigen::VectorXd& aVectEtaMu);

  Eigen::Vector3d mT; //Result Force
  double mDt; //Time delta
  Eigen::Vector3d mEta; //Pos vector
  Eigen::Vector3d mMu; //Speed vector

  Ship* mShip;  
};

#endif
