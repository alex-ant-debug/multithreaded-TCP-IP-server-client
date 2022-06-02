#include "TCP_IP/include/TcpServer.h"
#include "TCP_IP/include/Calculator.h"

#include <iostream>

using namespace tcp;
using namespace calc;

Calculator A;


TcpServer server(8081,
    {1, 1, 1}, // Keep alive{idle:1s, interval: 1s, pk_count: 1}

    [](ReceivedData data, TcpServer::Client& client){ // Data handler

        A.dataExchange(data, client);
    },

    [](TcpServer::Client& client) { // Connect handler
      std::cout << "Client " << A.getHostStr(client) << " connected\n";
    },

    [](TcpServer::Client& client) { // Disconnect handler
      std::cout << "Client " << A.getHostStr(client) << " disconnected\n";
    },

    std::thread::hardware_concurrency() // Thread pool size
);



int main() {

  try {
    //Start server
    if(server.start() == TcpServer::status::up) {
      std::cout<<"Server listen on port: " << server.getPort() << std::endl
               <<"Server handling thread pool size: " << server.getThreadPool().getThreadCount() << std::endl;
      server.joinLoop();
      return EXIT_SUCCESS;
    } else {
      std::cout<<"Server start error! Error code:"<< int(server.getStatus()) <<std::endl;
      return EXIT_FAILURE;
    }

  } catch(std::exception& except) {
    std::cerr << except.what();
    return EXIT_FAILURE;
  }
}
