#include "Engine.hpp"

Engine::Engine(void)
{

}

Engine::Engine(std::string aBrand, std::string aType, unsigned int aPower, float aRpmMax, float aFuelCons)
{
  Init(aBrand, aType, aPower, aRpmMax, aFuelCons);
}

Engine::~Engine(void)
{

}

void Engine::Init(std::string aBrand, std::string aType, unsigned int aPower, float aRpmMax, float aFuelCons)
{
  mBrand = aBrand;
  mType = aType;
  mPower = aPower;
  mRpmMax = aRpmMax;
  mFuelConsumption = aFuelCons;
}

float Engine::getRpmMax(void)
{
  return mRpmMax;
}
