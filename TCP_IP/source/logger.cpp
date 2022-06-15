#include "../include/logger.h"

using namespace logg;

std::string Logger::logLavelName[4] = {"INFO",
                                       "WARNING",
                                       "ERROR",
                                       "DEBUG"};

Logger::Logger(const std::string fileName,
               std::string prefix)
               :  fileName(fileName),
                  prefix(prefix)
                  {}

Logger::~Logger()
{
    std::string logText = os.str();

    if(!this->fileName.empty()) {
        std::ofstream out;
        out.open(this->fileName, std::ios::app);
        if(out.is_open()) {
            out << logText;
        }
        out.close();
    } else {
        fprintf(stderr, "%s", logText.c_str());
    }
    fflush(stderr);
}

std::ostringstream& Logger::Get(LOG_LEVEL logLevel)
{
    time_t t = time(nullptr);
    mut_consol.lock();
    os << std::put_time(localtime(&t), "%c") << ";";
    os << " " << logLavelName[logLevel] << "; ";
    os << this->prefix << "(" << (size_t)pthread_self() << "): ";
    mut_consol.unlock();
    return os;
}
