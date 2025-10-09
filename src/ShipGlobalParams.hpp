#ifndef SHIP_PARAMS_H
#define SHIP_PARAMS_H

#include <iostream>
#include <vector>

typedef struct{
  double lPP; //length  
  double b; //breadth
  double d; //draught
  double volume; //subwater volume 
  double xG; //Longitudinal coordinate of center of gravity of ship
  double cB; //Coefficient Block
}sGeoParams;

#endif
