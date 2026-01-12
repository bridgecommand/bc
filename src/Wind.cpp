#include "Wind.hpp"
#include "OwnShip.hpp"
#include "Constants.hpp"

Wind::Wind()
{

}

Wind::~Wind()
{

}

void Wind::load(OwnShip *aOwnShip)
{
  mOwnShip = aOwnShip;

}


void Wind::update(void)
{
  float windSpeedMps=0, windFlowDirection=0, relativeWindFlowDirection=0, axialWind=0, lateralWind=0;
  float relWindAxialMps=0, relWindLateralMps=0, frontalArea=0, sideArea=0, alpha=0;

  windSpeedMps = mWindSpeed * KTS_TO_MPS;
  windFlowDirection = mWindDirection + 180; //Wind direction is where the wind is from. We want where it is flowing towards
  relativeWindFlowDirection = windFlowDirection - mOwnShip->getHeading();
  axialWind = mWindSpeed * cos(relativeWindFlowDirection * PI/180);
  lateralWind = mWindSpeed * sin(relativeWindFlowDirection * PI/180);

  relWindAxialMps = (axialWind - mOwnShip->getSpeed()) * KTS_TO_MPS;
  relWindLateralMps = (lateralWind - mOwnShip->getLateralSpeed()) * KTS_TO_MPS;

  frontalArea = mOwnShip->getGeoParams().b * mOwnShip->getGeoParams().d; //TODO : Add an air draught param
  sideArea = mOwnShip->getGeoParams().lPP * mOwnShip->getGeoParams().d; //TODO : Add an air draught param

  mAxialWindDrag = -1 * pow(relWindAxialMps, 2) * (relWindAxialMps/abs(relWindAxialMps)) * 0.5 * RHO_AIR * frontalArea;
  mLateralWindDrag = -1 * pow(relWindLateralMps, 2) * relWindAxialMps/abs(relWindAxialMps) * 0.5 * RHO_AIR * sideArea;

  alpha = (mWindDirection - mOwnShip->getHeading()) * PI/180;
  mApparentWindSpd = sqrt(pow(mOwnShip->getSpeedThroughWater(), 2) + pow((mWindSpeed), 2) + (2 * mOwnShip->getSpeedThroughWater() * (mWindSpeed) * cos(alpha)));
  mApparentWindDir = atan2(mWindSpeed * sin(alpha), mOwnShip->getSpeedThroughWater() + mWindSpeed * cos(alpha));
  
}


void Wind::setTrueDirection(float aWindDirection){mWindDirection = aWindDirection;}
float Wind::getTrueDirection() const{return mWindDirection;}
void Wind::setTrueSpeed(float aWindSpeed){mWindSpeed = aWindSpeed;}
float Wind::getTrueSpeed() const{return mWindSpeed;}
float Wind::getApparentDir(void) const{return mApparentWindDir;}
float Wind::getApparentSpd(void) const{return mApparentWindSpd;}
float Wind::getAxialDrag(void) const{return mAxialWindDrag;}
float Wind::getLateralDrag(void) const{return mLateralWindDrag;}
