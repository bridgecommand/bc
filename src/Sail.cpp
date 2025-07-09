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

  Open(aPolarFile, aVarNameX, aVarNameY);
}

Sail::~Sail()
{
  nc_close(mIdPolarFile);
}

int Sail::Open(const std::string aPolarFile, std::string aVarNameX, std::string aVarNameY)
{
  if(!aPolarFile.empty() && !aVarNameX.empty() && !aVarNameY.empty())
    {
      int errRet = nc_open(aPolarFile.c_str(), NC_NOWRITE, &mIdPolarFile);
      if(errRet == NC_NOERR)
	{
	  errRet = nc_inq_varid(mIdPolarFile, aVarNameX.c_str(), &mSailVarX);
	  if(errRet != NC_NOERR)
	    {
	      nc_close(mIdPolarFile);
	    }
	  else
	    {
	      errRet = nc_inq_varndims(mIdPolarFile, mSailVarX, &mDimCountX);
	      if(errRet != NC_NOERR)
		{
		  nc_close(mIdPolarFile);
		}
	    }

	  errRet = nc_inq_varid(mIdPolarFile, aVarNameY.c_str(), &mSailVarY);
	  if(errRet != NC_NOERR)
	    {
	      nc_close(mIdPolarFile);
	    }
	  else
	    {
	      errRet = nc_inq_varndims(mIdPolarFile, mSailVarY, &mDimCountY);
	      if(errRet != NC_NOERR)
		{
		  nc_close(mIdPolarFile);
		}

	      return 0;
	    }
	}
    }

  return -1;
}

int Sail::Init(std::string aSpeedWaterVarName, std::string aWindSpeedVarName, std::string aWindAngleVarName)
{
  int err = -1;
  
  if(TOTAL_SAIL_DIM_COUNT == mDimCountX && TOTAL_SAIL_DIM_COUNT == mDimCountY)
    {
      int speedTroughWaterVar, trueWindSpeedVar, trueWindAngleVar;
      size_t stwSize, twsSize, twaSize; //2 last dims not used
      int dimIds[NC_MAX_VAR_DIMS];
      int nDims;

      nc_inq_varid(mIdPolarFile, aSpeedWaterVarName.c_str(), &speedTroughWaterVar);
      nc_inq_varid(mIdPolarFile, aWindSpeedVarName.c_str(), &trueWindSpeedVar);
      nc_inq_varid(mIdPolarFile, aWindAngleVarName.c_str(), &trueWindAngleVar);

      //speedThroughWaterVar
      nc_inq_varndims(mIdPolarFile, speedTroughWaterVar, &nDims);
      nc_inq_vardimid(mIdPolarFile, speedTroughWaterVar, dimIds);
      nc_inq_dimlen(mIdPolarFile, dimIds[0], &stwSize);

      //trueWindSpeedVar
      nc_inq_varndims(mIdPolarFile, trueWindSpeedVar, &nDims);
      nc_inq_vardimid(mIdPolarFile, trueWindSpeedVar, dimIds);
      nc_inq_dimlen(mIdPolarFile, dimIds[0], &twsSize);

      //trueWindAngleVar
      nc_inq_varndims(mIdPolarFile, trueWindAngleVar, &nDims);
      nc_inq_vardimid(mIdPolarFile, trueWindAngleVar, dimIds);
      nc_inq_dimlen(mIdPolarFile, dimIds[0], &twaSize);
      
      mStw.resize(stwSize);
      mTws.resize(twsSize);
      mTwa.resize(twaSize);

      /*Store data*/
      nc_get_var_float(mIdPolarFile, speedTroughWaterVar, mStw.data());
      nc_get_var_float(mIdPolarFile, trueWindSpeedVar, mTws.data());
      nc_get_var_float(mIdPolarFile, trueWindAngleVar, mTwa.data());

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
      nc_get_vara_float(mIdPolarFile, mSailVarX, start.data(), count.data(), &force);
    }
  else if('Y' == aAxe)
    {
      nc_get_vara_float(mIdPolarFile, mSailVarY, start.data(), count.data(), &force);
    }
      
  return force;
}
