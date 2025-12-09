#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>

class Engine
{

public:

  Engine();
  Engine(std::string aBrand, std::string aType, unsigned int aPower, float aRpmMax, float aFuelCons);
  ~Engine();

  void Init(std::string aBrand, std::string aType, unsigned int aPower, float aRpmMax, float aFuelCons);
  float getRpmMax(void);
  
private:

  std::string mBrand;
  std::string mType;
  unsigned int mPower; //kW
  float mRpmMax; //rpm
  float mFuelConsumption; //g/kWh 100%
};


#endif
