#ifndef __NETWORK_HPP_INCLUDED__
#define __NETWORK_HPP_INCLUDED__

#include <enet/enet.h>

//Forward declarations
class SimulationModel;

class Network
{
public:
    Network(SimulationModel* model);
    ~Network();

    void connectToServer();
    void update();

private:
    SimulationModel* model;

    ENetHost * client;
    ENetAddress address;
    ENetEvent event;
    ENetPeer *peer;

    void sendNetwork();
    void receiveNetwork();

};

#endif
