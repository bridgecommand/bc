#ifndef __TIME_HPP_INCLUDED__
#define __TIME_HPP_INCLUDED__



struct sTime{

  irr::u32 loopNumber; //u32 should be up to 4,294,967,295, so over 2 years at 60 fps
  irr::u32 currentTime; //Computer clock time
  irr::u32 previousTime; //Computer clock time
  float deltaTime;
  float scenarioTime; //Simulation internal time, starting at zero at 0000h on start day of simulation
  unsigned long long scenarioOffsetTime; //Simulation day's start time from unix epoch (1 Jan 1970)
  unsigned long long absoluteTime; //Unix timestamp for current time, including start day. Calculated from scenarioTime and scenarioOffsetTime
};


#endif
