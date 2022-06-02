#ifndef CALCULATOR_H_INCLUDED
#define CALCULATOR_H_INCLUDED

#include "TcpServer.h"
#include <iostream>
#include <sstream>
#include <map>


namespace calc {

class Calculator{

protected:

enum  commandIndex {
    ADD,
    SUB,
    MUL,
    DIV
};

std::map<std::string, int> commandList = {{"add", ADD},
                                          {"sub", SUB},
                                          {"mul", MUL},
                                          {"div", DIV},};



public:

Calculator();

~Calculator();

void sendingResponse(int32_t answer , tcp::TcpServer::Client& client);
std::string getHostStr(const tcp::TcpServer::Client& client);
void dataExchange(tcp::ReceivedData data, tcp::TcpServer::Client& client);
int commandProcessing(std::vector <std::string> data);
void messageParsing(std::string message, const char separator, std::vector <std::string> &out );
int32_t addition(int16_t a, int16_t b);
int32_t subtraction(int16_t a, int16_t b);
int32_t multiplication(int16_t a, int16_t b);
int32_t division(int16_t a, int16_t b);


};

}

#endif // CALCULATOR_H_INCLUDED
