#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include <fstream>
#include <string>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <sstream>
#include <mutex>


namespace logg {

typedef enum typelog {
    INFO,
    WARNING,
    ERROR,
    DEBUG
}LOG_LEVEL;


class Logger
{
public:
    Logger();
    Logger(std::string prefix);
    virtual ~Logger();

    std::ostringstream& Get(LOG_LEVEL logLevel = INFO);
    static void setName(std::string name);
    static std::string logLavelName[4];

protected:

    static std::string pathAndNameLogFile;
    std::string prefix;
    std::ostringstream os;
    std::mutex mut_consol;
};

}
#endif // LOGGER_H_INCLUDED
