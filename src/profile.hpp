#ifndef __PROFILE_HPP_INCLUDED__
#define __PROFILE_HPP_INCLUDED__


#include <chrono> //for profiling only
#include <iostream>

//Profiling
class Profiler {

private:
     std::chrono::steady_clock::time_point startBlock;
     //std::chrono::steady_clock::time_point endBlock;
     double elapsedSeconds;
     std::string name;


public:

    Profiler(std::string name) {
        elapsedSeconds = 0.0;
        this->name = name;
    }

    ~Profiler() {
        std::cout << "Elapsed time in " << name << ": " << elapsedSeconds << " s" << std::endl;
    }

    void tic() {
        startBlock = std::chrono::steady_clock::now();
    }

    void toc() {
        std::chrono::steady_clock::time_point endBlock = std::chrono::steady_clock::now();
        elapsedSeconds += std::chrono::duration_cast<std::chrono::microseconds>(endBlock - startBlock).count()/1e6;
    }

    double report() {
        std::cout << "Elapsed time in " << name << ": " << elapsedSeconds << " s" << std::endl;
        return elapsedSeconds;
    }

};

#endif
