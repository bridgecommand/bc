#include "Tide.hpp"

using namespace irr;

Tide::Tide()
{

}

Tide::~Tide()
{
    //dtor
}

void Tide::load(const std::string& worldName) {
    tideHeight = 0;

    //load tide.ini information
    std::string tideFilename = worldName;
    tideFilename.append("/tide.ini");

}

void Tide::update(irr::f32 scenarioTime) {

}

irr::f32 Tide::getTideHeight() {
    return tideHeight;
}

