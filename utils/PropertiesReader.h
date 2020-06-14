#ifndef PROPERTIES_READER
#define PROPERTIES_READER

#include <string>
#include <map>

class propertiesReader
{
public:
    explicit propertiesReader();
    void loadResource(const std::string& resource);
    std::string getProperty(const std::string& name);
private:
    std::map<std::string, std::string> store;
};

#endif
