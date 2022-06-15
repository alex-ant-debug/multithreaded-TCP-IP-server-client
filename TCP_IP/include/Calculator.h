#ifndef CALCULATOR_H_INCLUDED
#define CALCULATOR_H_INCLUDED

//#include "TcpServer.h"
#include <iostream>
#include <sstream>
#include <map>
#include <thread>
#include <vector>



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

std::string sendingResponse(int32_t answer);
std::string dataExchange(std::vector<uint8_t> data);
int commandProcessing(std::vector <std::string> data);
void messageParsing(std::string message, const char separator, std::vector <std::string> &out );
static int32_t addition(int16_t a, int16_t b);
int32_t subtraction(int16_t a, int16_t b);
int32_t multiplication(int16_t a, int16_t b);
int32_t division(int16_t a, int16_t b);

};

}

#endif // CALCULATOR_H_INCLUDED
