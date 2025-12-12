#ifndef SAIL_HPP
#define SAIL_HPP

#include <string>
#include <vector>
#include <netcdf.h>
#include <Eigen/Dense>
#include "irrlicht.h"

#define TOTAL_SAIL_DIM_COUNT (5)
#define SAILS_MAX (4)

class Sail
{
 public:

  Sail();
  Sail(const std::string aPolarFile, std::string aVarNameX, std::string aVarNameY);
  ~Sail();

  /*Sails*/
  void Init(int aSailsCount, std::string aSailsType, std::string aSailsSize, float (*aSailsPos)[3]);
  void SetMeshScene(irr::scene::IMeshSceneNode *aMeshScene);
  irr::scene::IMeshSceneNode* GetMeshScene(unsigned char aIndex);
  unsigned char GetCount(void);
  std::string GetType(void);
  std::string GetSize(void);
  float (*GetPos(void))[3];
  
  /*Polar file*/
  int OpenPolar(const std::string aPolarFile, std::string aVarNameX, std::string aVarNameY);
  int InitPolar(std::string aSpeedWaterVarName, std::string aWindSpeedVarName, std::string aWindAngleVarName);
  float GetForce(char aAxe, float aStwValue, float aTwsValue, float aTwaValue);

  /*Ship*/
  void ComputeT(void);
  void SetSTW(double aSpeedThroughWater);
  void SetWind(double aTrueWindSpeed, double aApparentWindDir);
  Eigen::Vector3d& getT(void);
  
 private:

  //Mesh Sails
  irr::scene::IMeshSceneNode* mSailsScene[SAILS_MAX];

  //Sail management
  int mSailsCount;
  std::string mSailsType;
  std::string mSailsSize;
  float mSailsPos[SAILS_MAX][3];

  //Compute force and manage nc file
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
