#include "../include/logger.h"

using namespace logg;

std::string Logger::logLavelName[4] = {"INFO",
                                       "WARNING",
                                       "ERROR",
                                       "DEBUG"};


Logger::Logger(){
    prefix = "";
}

Logger::Logger(std::string prefix) :
                  prefix(prefix)
                  {}

Logger::~Logger()
{
    mut_consol.lock();
    std::string logText = os.str();

    if(!this->pathAndNameLogFile.empty()) {
        std::ofstream out;
        out.open(this->pathAndNameLogFile, std::ios::app);
        if(out.is_open()) {
            out << logText;
        }
        out.close();
    } else {
        fprintf(stderr, "%s", logText.c_str());
    }
    fflush(stderr);
    mut_consol.unlock();
}

std::ostringstream& Logger::Get(LOG_LEVEL logLevel)
{
    std::lock_guard lock(mut_consol);
    time_t t = time(nullptr);
    os << std::put_time(localtime(&t), "%c") << ";";
    os << " " << logLavelName[logLevel] << "; ";
    os << this->prefix << "(" << (size_t)pthread_self() << "): ";
    return os;
}

void Logger::setName(std::string name)
{
    Logger::pathAndNameLogFile = name;
}
