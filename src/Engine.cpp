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

void Engine::PrintParams(void)
{
  std::cout << "::::::Engine Parameters::::::" << std::endl;
  std::cout << "Brand : " << mBrand << std::endl;
  std::cout << "Type : " << mType << std::endl;
  std::cout << "Power (kW): " << mPower << std::endl;
  std::cout << "Rpm max (rpm): " << mRpmMax << std::endl;
  std::cout << "Fuel Consumption (g/kWh 100%) : " << mFuelConsumption << std::endl;
  std::cout << "::::::::::::" << std::endl;
}

float Engine::getRpmMax(void)
{
  return mRpmMax;
}
