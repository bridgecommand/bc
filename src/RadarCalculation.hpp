/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2014 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#ifndef __RADARCALCULATION_HPP_INCLUDED__
#define __RADARCALCULATION_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>
#include <string>
#include <stdint.h> //for uint64_t

#include <ctime> //To check time elapsed between changing EBL when button held down

class Terrain;
class OwnShip;
class Buoys;
class OtherShips;
struct RadarData;

enum ARPA_CONTACT_TYPE {
    CONTACT_NONE,
    CONTACT_NORMAL,
    CONTACT_MANUAL
};

struct ARPAScan {
    //ARPA scan information based on the assumption that the own ship position is well known
    float x; //Absolute metres
    float z; //Absolute metres
    uint64_t timeStamp; //Timestamp in seconds
    //float estimatedRCS; //Estimated radar cross section
    float rangeNm; //Reference only
    float bearingDeg; //For reference only

    ARPAScan() {
        x = 0;
        z = 0;
        timeStamp = 0;
        rangeNm = 0;
        bearingDeg = 0;
    }
};

struct ARPAEstimatedState {
    uint32_t displayID; //User displayed ID
    bool stationary; // E.g. if detected as static and a small RCS or a buoy.
    float absVectorX; //Estimated X speed (m/s)
    float absVectorZ; //Estimated Z speed (m/s)
    float absHeading; //Estimated heading (deg)
    float relVectorX; //Estimated X speed (m/s)
    float relVectorZ; //Estimated Z speed (m/s)
    float relHeading; //Estimated heading (deg)
    float range; //Estimated current range from own ship (Nm)
    float bearing; //Estimated current bearing from own ship (deg)
    float speed; //Estimated current speed (Kts)
    bool lost; //True if we last scanned more than a defined time ago.
    float cpa; //Closest point of approach in Nm
    float tcpa; //Time to closest point of approach (mins)
    ARPA_CONTACT_TYPE contactType; //Duplicate of what's in the parent, but useful to pass to the GUI

    ARPAEstimatedState() {
        displayID = 0;
        stationary = false;
        absVectorX = 0;
        absVectorZ = 0;
        absHeading = 0;
        relVectorX = 0;
        relVectorZ = 0;
        relHeading = 0;
        range = 0;
        bearing = 0;
        speed = 0;
        lost = false;
        cpa = 0;
        tcpa = 0;
        contactType = CONTACT_NONE;    
    }
};

struct ARPAContact {
    std::vector<ARPAScan> scans;
    float totalXMovementEst; //Estimates of total movement (sum of absolutes) in X and Z, to help detect stationary contacts
    float totalZMovementEst;
    ARPA_CONTACT_TYPE contactType;
    void* contact;
    //uint32_t displayID;
    ARPAEstimatedState estimate;

    ARPAContact() {
        totalXMovementEst = 0;
        totalZMovementEst = 0;
        contactType = CONTACT_NONE;
        contact = 0;
    } 
};

class RadarCalculation
{
    public:
        RadarCalculation();
        virtual ~RadarCalculation();
        void load(std::string radarConfigFile, irr::IrrlichtDevice* dev);
        void increaseRange();
        void decreaseRange();
        float getRangeNm() const;
        void setGain(float value);
        void setClutter(float value);
        void setRainClutter(float value);
        void increaseClutter(float value);
        void decreaseClutter(float value);
        void increaseRainClutter(float value);
        void decreaseRainClutter(float value);
        void increaseGain(float value);
        void decreaseGain(float value);
        float getGain() const;
        float getClutter() const;
        float getRainClutter() const;
        float getEBLRangeNm() const;
        float getEBLBrg() const;
        float getCursorRangeNm() const;
        float getCursorBrg() const;
        void setPIData(int32_t PIid, float PIbearing, float PIrange);
        float getPIbearing(int32_t PIid) const;
        float getPIrange(int32_t PIid) const;
        void increaseCursorRangeXNm();
        void decreaseCursorRangeXNm();
        void increaseCursorRangeYNm();
        void decreaseCursorRangeYNm();
        void increaseEBLRange();
        void decreaseEBLRange();
        void increaseEBLBrg();
        void decreaseEBLBrg();
        void setNorthUp();
        void setCourseUp();
        void setHeadUp();
        bool getHeadUp() const; //Head or course up
		void toggleRadarOn();
        bool isRadarOn() const;
		int getArpaMode() const;
        void setArpaMode(int mode);
        void setRadarARPARel();
        void setRadarARPATrue();
        void setArpaListSelection(int32_t selection);
        int32_t getArpaListSelection() const;
        void setRadarARPAVectors(float vectorMinutes);
        void setRadarDisplayRadius(uint32_t radiusPx);
        void changeRadarColourChoice();
        uint32_t getARPATracksSize() const;
        int getARPAContactIDFromTrackIndex(uint32_t trackIndex) const;
        ARPAContact getARPAContactFromTrackIndex(uint32_t trackIndex) const;
        void addManualPoint(bool newContact, irr::core::vector3d<int64_t> offsetPosition, const OwnShip& ownShip, uint64_t absoluteTime);
        void clearManualPoints();
        void trackTargetFromCursor();
        void clearTargetFromCursor();
        irr::video::SColor getRadarForegroundColour() const;
        irr::video::SColor getRadarBackgroundColour() const;
        irr::video::SColor getRadarSurroundColour() const;
        void update(irr::video::IImage * radarImage, irr::video::IImage * radarImageOverlaid, irr::core::vector3d<int64_t> offsetPosition, const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips, float weather, float rain, float tideHeight, float deltaTime, uint64_t absoluteTime, irr::core::vector2di mouseRelPosition, bool isMouseDown);

    private:
        irr::IrrlichtDevice* device;
        std::vector<std::vector<float> > scanArray;
        std::vector<std::vector<float> > scanArrayAmplified;
        std::vector<std::vector<float> > scanArrayToPlot;
        std::vector<std::vector<float> > scanArrayToPlotPrevious;
        std::vector<bool> toReplot;
        std::vector<ARPAContact> arpaContacts;
        std::vector<uint32_t> arpaTracks;
        bool radarOn;
        int arpaMode; // 0: Off/Manual, 1: MARPA, 2: ARPA
        int32_t arpaListSelection;
        float radarGain;
        float radarRainClutterReduction;
        float radarSeaClutterReduction;
        float currentScanAngle;
        float scanAngleStep;
        uint32_t currentScanLine; //Note that this MUST be an integer, as the scanline number is used to look up values in radar scan arrays
        uint32_t rangeResolution;
        uint32_t angularResolution;
        float rangeSensitivity; //Used for ARPA contacts - in metres
        uint32_t radarRangeIndex;
        float radarScannerHeight;
        //parameters for noise behaviour
        float radarNoiseLevel;
        float radarSeaClutter;
        float radarRainClutter;
        //Parameters for parallel index
        std::vector<float> piBearings;
        std::vector<float> piRanges;
        //Parameters for EBL
        float EBLRangeNm;
        float EBLBrg;
        clock_t radarCursorsLastUpdated;
        //Parameters for radar cursor
        float cursorRangeXNm;
        float cursorRangeYNm;
        float CursorRangeNm;
        float CursorBrg;
        //Radar config
        bool headUp;
        bool stabilised;
        uint32_t radarRadiusPx;
        bool radarScreenStale;
        bool trueVectors;
        float vectorLengthMinutes;

        //colours
        std::vector<irr::video::SColor> radarBackgroundColours;
        std::vector<irr::video::SColor> radarForegroundColours;
        std::vector<irr::video::SColor> radarSurroundColours;
        uint32_t currentRadarColourChoice;

        std::vector<float> radarRangeNm;
        void scan(irr::core::vector3d<int64_t> offsetPosition, const Terrain& terrain, const OwnShip& ownShip, const Buoys& buoys, const OtherShips& otherShips, float weather, float rain, float tideHeight, float deltaTime, uint64_t absoluteTime);
        void updateARPA(irr::core::vector3d<int64_t> offsetPosition, const OwnShip& ownShip, uint64_t absoluteTime);
        void updateArpaEstimate(ARPAContact& thisArpaContact, int contactID, const OwnShip& ownShip, irr::core::vector3d<int64_t> absolutePosition, uint64_t absoluteTime);
        float radarNoise(float radarNoiseLevel, float radarSeaClutter, float radarRainClutter, float weather, float radarRange,float radarBrgDeg, float windDirectionDeg, float radarInclinationAngle, float rainIntensity);
        void render(irr::video::IImage * radarImage, irr::video::IImage * radarImageOverlaid, float ownShipHeading, float ownShipSpeed);
        float rangeAtAngle(float checkAngle,float centreX, float centreZ, float heading);
        void drawSector(irr::video::IImage * radarImage,float centreX, float centreY, float innerRadius, float outerRadius, float startAngle, float endAngle, uint32_t alpha, uint32_t red, uint32_t green, uint32_t blue, float ownShipHeading);
        void drawLine(irr::video::IImage * radarImage, float startX, float startY, float endX, float endY, uint32_t alpha, uint32_t red, uint32_t green, uint32_t blue);//Try with f32 as inputs so we can do interpolation based on the theoretical start and end
        void drawCircle(irr::video::IImage * radarImage, float centreX, float centreY, float radius, uint32_t alpha, uint32_t red, uint32_t green, uint32_t blue);//Try with f32 as inputs so we can do interpolation based on the theoretical start and end
        bool isPointInEllipse(float pointX, float pointZ, float centreX, float centreZ, float width, float length, float angle);

};

#endif
