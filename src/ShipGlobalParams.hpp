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

typedef struct{
  double mpX; //Added masses of x axis direction and y axis direction, respectively
  double mpY;
  double jpZ; //Added moment of inertia
}sAddedMassParams;

#endif
