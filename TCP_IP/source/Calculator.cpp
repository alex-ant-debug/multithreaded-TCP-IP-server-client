#include "../include/Calculator.h"
#include <stdio.h>
#include <cstring>

#define   INT_TO_STRING(in)  std::to_string(int(reinterpret_cast<char*>in))

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


void Calculator::sendingResponse(int32_t answer, tcp::TcpServer::Client& client)
{
    std::string sendAnswer = "Answer = " + std::to_string(answer);
    client.sendData(sendAnswer.c_str(), sizeof(sendAnswer));
    std::cout << "Client "<< getHostStr(client) <<" send data [ " << sendAnswer.size() << " bytes ]: " << sendAnswer.c_str() << '\n';
}

void Calculator::dataExchange(tcp::ReceivedData data, tcp::TcpServer::Client& client)
{
    std::vector <std::string> dataF;
    messageParsing((char*)data.data(), ' ', dataF);
    int command = commandProcessing(dataF);

    switch (command) {
        case ADD: {
            int32_t answer = addition(std::stoi(dataF.at(1)), std::stoi(dataF.at(2)));
            sendingResponse(answer, client);
            break;}
        case SUB: {
            int32_t answer = subtraction(std::stoi(dataF.at(1)), std::stoi(dataF.at(2)));
            sendingResponse(answer, client);
        break;}
        case MUL: {
            int32_t answer = multiplication(std::stoi(dataF.at(1)), std::stoi(dataF.at(2)));
            sendingResponse(answer, client);
        break;}
        case DIV: {
            int32_t answer = division(std::stoi(dataF.at(1)), std::stoi(dataF.at(2)));
            sendingResponse(answer, client);
        break;}
        default: {  std::cout << "Input Error "; break;}
    }
}

std::string Calculator::getHostStr(const tcp::TcpServer::Client& client) {
    uint32_t ip = client.getHost ();
    return std::string() + INT_TO_STRING((&ip)[0]) + '.' +
                           INT_TO_STRING((&ip)[1]) + '.' +
                           INT_TO_STRING((&ip)[2]) + '.' +
                           INT_TO_STRING((&ip)[3]) + ':' + std::to_string( client.getPort ());
}
