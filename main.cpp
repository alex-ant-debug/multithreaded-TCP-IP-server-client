#include "TCP_IP/include/TcpServer.h"

#include <iostream>
#include <sstream>
#include <algorithm>    // std::find
#include <string>
#include <map>

using namespace tcp;

//Parse ip to std::string
std::string getHostStr(const TcpServer::Client& client) {
    uint32_t ip = client.getHost ();
    return std::string() + std::to_string(int(reinterpret_cast<char*>(&ip)[0])) + '.' +
            std::to_string(int(reinterpret_cast<char*>(&ip)[1])) + '.' +
            std::to_string(int(reinterpret_cast<char*>(&ip)[2])) + '.' +
            std::to_string(int(reinterpret_cast<char*>(&ip)[3])) + ':' +
            std::to_string( client.getPort ());
}

void messageParsing(std::string message, const char separator, std::vector <std::string> &out )
{
    // create a stream from the string
    std::stringstream streamString(message);

    std::string intermediateString;
    while (std:: getline (streamString, intermediateString, separator) )
    {
        out.push_back(intermediateString); // store the string in intermediateString
    }
}



int commandProcessing(std::vector <std::string> data)
{
    std::map<std::string, int> commandList = {{"add", 0},
                                              {"sub", 1},
                                              {"mul", 2},
                                              {"div", 3},};

    auto item = commandList.find(data.at(0));

    if (item != commandList.end()) {

        return item->second;

    } else {
        return -1;
    }
}

int32_t addition(int16_t a, int16_t b)
{
    return a + b;
}

int32_t subtraction(int16_t a, int16_t b)
{
    return a - b;
}

int32_t multiplication(int16_t a, int16_t b)
{
    return a * b;
}

int32_t division(int16_t a, int16_t b)
{
    return a / b;
}

void dataExchange(ReceivedData data, TcpServer::Client& client)
{
    std::vector <std::string> dataF;
    messageParsing((char*)data.data(), ' ', dataF);
    int command = commandProcessing(dataF);
    std::string sendAnswer = "Hello, client!\n";

    switch (command) {
        case 0: {
            int32_t answer = addition(std::stoi(dataF.at(1)), std::stoi(dataF.at(2)));

            sendAnswer = "Adding two numbers = " + std::to_string(answer);
            client.sendData(sendAnswer.c_str(), sizeof(sendAnswer));
            std::cout << "Client "<< getHostStr(client) <<" send data [ " << sendAnswer.size() << " bytes ]: " << sendAnswer.c_str() << '\n';
            break;}
        case 1: {
            int32_t answer = subtraction(std::stoi(dataF.at(1)), std::stoi(dataF.at(2)));

            sendAnswer = "Subtraction two numbers = " + std::to_string(answer);
            client.sendData(sendAnswer.c_str(), sizeof(sendAnswer));
            std::cout << "Client "<< getHostStr(client) <<" send data [ " << sendAnswer.size() << " bytes ]: " << sendAnswer.c_str() << '\n';
        break;}
        case 2: {
            int32_t answer = multiplication(std::stoi(dataF.at(1)), std::stoi(dataF.at(2)));

            sendAnswer = "Multiplication two numbers = " + std::to_string(answer);
            client.sendData(sendAnswer.c_str(), sizeof(sendAnswer));
            std::cout << "Client "<< getHostStr(client) <<" send data [ " << sendAnswer.size() << " bytes ]: " << sendAnswer.c_str() << '\n';
        break;}
        case 3: {
            int32_t answer = division(std::stoi(dataF.at(1)), std::stoi(dataF.at(2)));

            sendAnswer = "Division two numbers = " + std::to_string(answer);
            client.sendData(sendAnswer.c_str(), sizeof(sendAnswer));
            std::cout << "Client "<< getHostStr(client) <<" send data [ " << sendAnswer.size() << " bytes ]: " << sendAnswer.c_str() << '\n';
        break;}
        default: {  std::cout << "Input Error "; break;}
    }
}

TcpServer server(8081,
{1, 1, 1}, // Keep alive{idle:1s, interval: 1s, pk_count: 1}

[](ReceivedData data, TcpServer::Client& client){ // Data handler

    dataExchange(data, client);

  //client.sendData(sendAnswer.c_str(), sizeof(sendAnswer));
},

[](TcpServer::Client& client) { // Connect handler
  std::cout << "Client " << getHostStr(client) << " connected\n";
},

[](TcpServer::Client& client) { // Disconnect handler
  std::cout << "Client " << getHostStr(client) << " disconnected\n";
},

std::thread::hardware_concurrency() // Thread pool size
);



int main() {
  //using namespace std::chrono_literals;
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
