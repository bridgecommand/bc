
#ifndef __WIND_HPP_INCLUDED__
#define __WIND_HPP_INCLUDED__

class OwnShip;


class Wind
{
public:
  Wind();
  virtual ~Wind();

  void load(OwnShip *aOwnShip);
  void update(void);
  
  void setTrueDirection(float aWindDirection);
  float getTrueDirection() const;
  void setTrueSpeed(float aWindSpeed); 
  float getTrueSpeed() const;
  float getApparentDir(void) const;
  float getApparentSpd(void) const;
  float getAxialDrag(void) const;
  float getLateralDrag(void) const;
  
private:

  OwnShip *mOwnShip;
  float mWindDirection; //0-360
  float mWindSpeed; //Nm
  float mApparentWindDir;
  float mApparentWindSpd;
  float mWindFlowDirection;
  float mAxialWindDrag;
  float mLateralWindDrag;        
};

#endif





 
