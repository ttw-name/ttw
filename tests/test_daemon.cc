
#include <string>
#include "daemon.h"
#include <iostream>
void hand_signal(){

}

int main(int argc, char* argv[]){

    ttw::Signal::signal(SIGINT, hand_signal);
    if(argc != 2){
        return 1;
    }
    std::string program = argv[0];
    std::string pidfile = program + ".pid";
    std::cout << "program : "  << program << " pidfile : " << pidfile << std::endl; 
    ttw::Daemon::daemonProcess(argv[1], pidfile.c_str());
    while(1){
        sleep(5);
    }
    return 1;
}