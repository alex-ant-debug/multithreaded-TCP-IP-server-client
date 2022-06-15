#include "../include/Calculator.h"
#include <stdio.h>
#include <cstring>


using namespace calc;


Calculator::Calculator(){

}

Calculator::~Calculator(){

}

void Calculator::messageParsing(std::string message, const char separator, std::vector <std::string> &out )
{
    // create a stream from the string
    std::stringstream streamString(message);

    std::string intermediateString;
    while (std:: getline (streamString, intermediateString, separator) )
    {
        out.push_back(intermediateString); // store the string in intermediateString
    }
}


int Calculator::commandProcessing(std::vector <std::string> data)
{
    auto item = commandList.find(data.at(0));

    if (item != commandList.end()) {

        return item->second;

    } else {
        return -1;
    }
}

int32_t Calculator::addition(int16_t a, int16_t b)
{
    return a + b;
}

int32_t Calculator::subtraction(int16_t a, int16_t b)
{
    return a - b;
}

int32_t Calculator::multiplication(int16_t a, int16_t b)
{
    return a * b;
}

int32_t Calculator::division(int16_t a, int16_t b)
{
    return a / b;
}


std::string Calculator::sendingResponse(int32_t answer)
{
    return "Answer = " + std::to_string(answer);
}

std::string Calculator::dataExchange(std::vector<uint8_t> data)
{
    std::string sendingAnswer;
    std::vector <std::string> dataF;
    messageParsing((char*)data.data(), ' ', dataF);
    int command = commandProcessing(dataF);

    switch (command)
    {
        case ADD:
        {
            int32_t answer =  addition(std::stoi(dataF.at(1)), std::stoi(dataF.at(2)));
            sendingAnswer = sendingResponse(answer);
        }
        break;
        case SUB:
        {
            int32_t answer =  subtraction(std::stoi(dataF.at(1)), std::stoi(dataF.at(2)));
            sendingAnswer = sendingResponse(answer);
        }
        break;
        case MUL:
        {
            int32_t answer =  multiplication(std::stoi(dataF.at(1)), std::stoi(dataF.at(2)));
            sendingAnswer = sendingResponse(answer);
        }
        break;
        case DIV:
        {
            int32_t answer =  division(std::stoi(dataF.at(1)), std::stoi(dataF.at(2)));
            sendingAnswer = sendingResponse(answer);
        }
        break;
        default: {  /*std::cout << "Input Error ";*/ break;}
    }
    return sendingAnswer;
}
