/*   Bridge Command 5.0 - MergeToolSocket Integration
     Copyright (C) 2016 Dierk Brauer
     dierk.brauer@uni-oldenburg.de
*/

#ifndef __MERGETOOLSOCKET_HPP_INCLUDED__
#define __MERGETOOLSOCKET_HPP_INCLUDED__

#include <string>

//Forward declarations
class SimulationModel;

class MergeToolSocket
{
public:
    MergeToolSocket();
    ~MergeToolSocket();

    void setModel(SimulationModel* model);
    void update();

private:
    const std::string CMD_UPDATE_ENV_ELEM = "UPDATE_ENV_ELEM";
    int newsockfd, sockfd;
    SimulationModel* model;
    void send();
    void receive();
    bool startServer(int port);
    void error(const char *msg);
};

#endif
