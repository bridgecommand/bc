#include "Sail.hpp"
#include <iostream>
#include <cmath>
#include <vector>

Sail::Sail(void)
{
  mDimCountX = 0;
  mDimCountY = 0;
}

Sail::Sail(const std::string aPolarFile, std::string aVarNameX, std::string aVarNameY)
{
  mDimCountX = 0;
  mDimCountY = 0;
  
  if(!aPolarFile.empty() && !aVarNameX.empty() && !aVarNameY.empty())
    {    
      mPolarFile.open(aPolarFile, NcFile::read);
      mSailVarX = mPolarFile.getVar(aVarNameX);
      mDimCountX = mSailVarX.getDimCount();

      mSailVarY = mPolarFile.getVar(aVarNameY);
      mDimCountY = mSailVarY.getDimCount();
    }
}

Sail::~Sail()
{
  mPolarFile.close();
}

int Sail::Open(const std::string aPolarFile, std::string aVarNameX, std::string aVarNameY)
{
  if(!aPolarFile.empty() && !aVarNameX.empty() && !aVarNameY.empty())
    {    
      mPolarFile.open(aPolarFile, NcFile::read);
      mSailVarX = mPolarFile.getVar(aVarNameX);
      mDimCountX = mSailVarX.getDimCount();

      mSailVarY = mPolarFile.getVar(aVarNameY);
      mDimCountY = mSailVarY.getDimCount();

      return 0;
    }
  return -1;
}

int Sail::Init(std::string aSpeedWaterVarName, std::string aWindSpeedVarName, std::string aWindAngleVarName)
{
  int err = -1;
  
  if(TOTAL_SAIL_DIM_COUNT == mDimCountX && TOTAL_SAIL_DIM_COUNT == mDimCountY)
    {
      /*Get variables from file*/
      NcVar speedTroughWaterVar = mPolarFile.getVar(aSpeedWaterVarName);
      NcVar trueWindSpeedVar = mPolarFile.getVar(aWindSpeedVarName);
      NcVar trueWindAngleVar = mPolarFile.getVar(aWindAngleVarName);

      /*Get size for each dimension*/
      size_t stwSize = speedTroughWaterVar.getDim(0).getSize();
      size_t twsSize = trueWindSpeedVar.getDim(0).getSize();
      size_t twaSize = trueWindAngleVar.getDim(0).getSize();
      //2 last dims not used

      mStw.resize(stwSize);
      mTws.resize(twsSize);
      mTwa.resize(twaSize);

      /*Store data*/
      speedTroughWaterVar.getVar(mStw.data());
      trueWindSpeedVar.getVar(mTws.data());
      trueWindAngleVar.getVar(mTwa.data());
      
      err=0;
    }

  return err;
}


size_t FindClosestIndex(const std::vector<float>& aValue, float aTarget)
{
    size_t best = 0;
    float minDiff = std::abs(aValue[0] - aTarget);

    for(size_t i = 1; i < aValue.size(); ++i)
      {
        float diff = std::abs(aValue[i] - aTarget);
        if(diff < minDiff)
	  {
            minDiff = diff;
            best = i;
	  }
    }
    return best;
}


float Sail::GetForce(char aAxe, float aStwValue, float aTwsValue, float aTwaValue)
{
  float force = 0;  

  size_t iStw = FindClosestIndex(mStw, aStwValue);
  size_t iTws = FindClosestIndex(mTws, aTwsValue);
  size_t iTwa = FindClosestIndex(mTwa, aTwaValue);

  std::vector<size_t> start = {iStw, iTws, iTwa, 0, 0};
  std::vector<size_t> count = {1, 1, 1, 1, 1};

  if('X' == aAxe)
    {
      mSailVarX.getVar(start, count, &force);
    }
  else if('Y' == aAxe)
    {
      mSailVarY.getVar(start, count, &force);
    }
      
  return force;
}
