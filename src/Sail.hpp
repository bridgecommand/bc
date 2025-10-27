#ifndef SAIL_HPP
#define SAIL_HPP

#include <string>
#include <vector>
#include <netcdf.h>
#include <Eigen/Dense>

#define TOTAL_SAIL_DIM_COUNT (5)

class Sail
{
 public:

  Sail();
  Sail(const std::string aPolarFile, std::string aVarNameX, std::string aVarNameY);
  ~Sail();
  
  /*Polar file*/
  int Open(const std::string aPolarFile, std::string aVarNameX, std::string aVarNameY);
  int Init(std::string aSpeedWaterVarName, std::string aWindSpeedVarName, std::string aWindAngleVarName);
  float GetForce(char aAxe, float aStwValue, float aTwsValue, float aTwaValue);

  /*Ship*/
  void ComputeT(void);
  void SetSTW(double aSpeedThroughWater);
  void SetWind(double aTrueWindSpeed, double aApparentWindDir);
  Eigen::Vector3d& getT(void);
  
 private:

  int mIdPolarFile;

  int mSailVarX;
  int mDimCountX;
  int mSailVarY;
  int mDimCountY;

  std::vector<float> mStw;
  std::vector<float> mTws;
  std::vector<float> mTwa;

  double mSpeedThroughWater;
  double mTrueWindSpeed;
  double mApparentWindDir;
  
  Eigen::Vector3d mT; //Sail force generated 
};




#endif
