#include "TCP_IP/include/TcpServer.h"
#include <iostream>
#include "TCP_IP/include/logger.h"


using namespace tcp;
using namespace logg;


TcpServer server(8081,
                // Keep alive{1s, 1s, 1}
                {1, 1, 1},

                // Data handler
                [](ReceivedData data, TcpServer::Client& client){

                    std::string sendAnswer = client.primitiveComputing.dataExchange(data);
                    if(!sendAnswer.empty()) {
                        Logger("", "").Get() << "Client "<< client.getHostStr(client) <<" send data [ " << sendAnswer.size() << " bytes ]: " << sendAnswer.c_str() << '\n';
                    }
                },

                // Connect handler
                [](TcpServer::Client& client) {
                    Logger("", "").Get() << "Client " << client.getHostStr(client) << " connected\n";
                },

                // Disconnect handler
                [](TcpServer::Client& client) {
                    Logger("", "").Get() <<  "Client " << client.getHostStr(client) << " disconnected\n";
                },

                // Thread pool size
                std::thread::hardware_concurrency()
);



int main(int argc, char *argv[]) {

/*std::cout << "There are " << argc << " arguments:\n";

    // Перебираем каждый аргумент и выводим его порядковый номер и значение
    for (int count=0; count < argc; ++count)
        std::cout << count << " " << argv[count] << '\n';*/



  try {
    //Start server
    if(server.start() == TcpServer::status::up) {
      Logger("", "").Get() << "Server listen on port: " << server.getPort() << std::endl
                           << "Server handling thread pool size: " << server.getThreadPool().getThreadCount() << std::endl;
      server.joinLoop();
      return EXIT_SUCCESS;
    } else {
      Logger("", "").Get(ERROR) << "Server start error! Error code:"<< int(server.getStatus()) <<std::endl;
      return EXIT_FAILURE;
    }

  } catch(std::exception& except) {
    std::cerr << except.what();
    return EXIT_FAILURE;
  }
}
