#include "PropertiesReader.h"


#include <sstream>
#include <fstream>


std::string& trim(std::string &s)
{
    if (s.empty()) return s;
    s.erase(0,s.find_first_not_of(' '));
    s.erase(s.find_last_not_of(' ') + 1);
    return s;
}

propertiesReader::propertiesReader() = default;

void propertiesReader::loadResource(const std::string& resource)
{
    std::ifstream stream;
    stream.open(resource);
    if(stream.is_open())
    {
        std::string buf;
        while(std::getline(stream, buf))
        {
            std::string key, value;
            std::istringstream is_line(buf);
            if(std::getline(is_line, key, '='))
            {
                if(std::getline(is_line, value))
                {
                    store[trim(key)] = trim(value);
                }
            }
        }
    }
    stream.close();
}

std::string propertiesReader::getProperty(const std::string& name)
{
    return store[name];
}


