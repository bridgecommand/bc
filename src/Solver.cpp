#include <cmath>
#include "Solver.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif // M_PI

Solver::Solver()
{
  mT << 0, 0, 0;
  mEta << 0, 0, 0;
  mMu << 0, 0, 0;
  mDt = 0;
  mShip = NULL;
}

Solver::Solver(Ship* aShip)
{
  mT << 0, 0, 0;
  mEta << 0, 0, 0;
  mMu << 0, 0, 0;
  mDt = 0;
  Init(aShip);
}


int Solver::Init(Ship* aShip)
{
  int ret=0;
  
  if(NULL != aShip)
    mShip = aShip;
  else
    ret=-1;

  return ret;
}

Eigen::VectorXd Solver::DiffEq(const Eigen::VectorXd& aVectEtaMu)
{
  const Eigen::Vector3d vEta(aVectEtaMu[0],aVectEtaMu[1],aVectEtaMu[2]);
  const Eigen::Vector3d vMu(aVectEtaMu[3],aVectEtaMu[4],aVectEtaMu[5]);
  Eigen::Matrix3d matRotPsi;
  Eigen::Matrix3d matC;
  Eigen::Vector3d vEtaP;
  Eigen::Vector3d vMuP;
  Eigen::VectorXd catVect;

  double xG = 0; 

  matRotPsi << cos(vEta[2]), -sin(vEta[2]) , 0,
    sin(vEta[2]), cos(vEta[2]), 0,
    0, 0 , 1;
  
  vEtaP = matRotPsi * vMu;

  xG = mShip->getGeoParams().xG;
  matC << 0, -(mShip->getM() + mShip->getMY())*vMu[2], -xG*mShip->getM()*vMu[2],
    (mShip->getM() + mShip->getMX())*vMu[2], 0, 0, xG*mShip->getM()*vMu[2], 0, 0;
  
  mShip->getHull().ComputeT(vMu, mShip->getRho(), mShip->getGeoParams());
  mShip->getPropeller().ComputeT(vMu, mShip->getRho(), mShip->getGeoParams());
  mShip->getRudder().ComputeT(vMu, mShip->getRho(), mShip->getGeoParams(), mShip->getPropeller());

  mT << mShip->getHull().getT() + mShip->getPropeller().getT() + mShip->getRudder().getT();
  
  if(mShip->getSail().GetCount() > 0)
    {
      mShip->getSail().ComputeT();
      mT += mShip->getSail().getT();
    }

  
  vMuP = mShip->getInvMatM() * (mT - (matC * vMu));
  
  catVect.resize(vEtaP.size() + vMuP.size());
  catVect << vEtaP, vMuP;
  
  return catVect;
}

void Solver::SetDeltaT(double aDt)
{
  mDt = aDt;
}

void Solver::SolveRk4(sTime& aTime, Eigen::Vector3d aEta, Eigen::Vector3d aMu)
{
  float aDt = aTime.deltaTime;
  
  Eigen::VectorXd y(VECTOR_SIZE_DIFF_EQ);
  Eigen::VectorXd tmp(VECTOR_SIZE_DIFF_EQ);
  Eigen::VectorXd dy1(VECTOR_SIZE_DIFF_EQ);
  Eigen::VectorXd dy2(VECTOR_SIZE_DIFF_EQ);
  Eigen::VectorXd dy3(VECTOR_SIZE_DIFF_EQ);
  Eigen::VectorXd dy4(VECTOR_SIZE_DIFF_EQ);
  Eigen::VectorXd ySol(VECTOR_SIZE_DIFF_EQ);
  
  SetDeltaT(aDt);
  y << aEta, aMu;

  dy1 = DiffEq(y);
  tmp = y + (0.5 * mDt * dy1);

  dy2 = DiffEq(tmp);
  tmp = y + (0.5 * mDt * dy2);

  dy3 = DiffEq(tmp);
  tmp = y + mDt * dy3;

  dy4 = DiffEq(tmp);
  ySol = y + mDt * (dy1 + 2*dy2 + 2*dy3 + dy4)/6;

  if(ySol[2] > (2*M_PI))
    ySol[2] = ySol[2] - (2*M_PI);

   if(ySol[2] < 0)
    ySol[2] = ySol[2] + (2*M_PI);
  
  mEta << ySol[0], ySol[1], ySol[2];
  mMu << ySol[3], ySol[4], ySol[5];

}

Eigen::Vector3d Solver::getEta(void) const {return mEta;}
Eigen::Vector3d Solver::getMu(void) const {return mMu;}
