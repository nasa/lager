#ifndef KEG
#define KEG

#include <string>
#include <vector>
#include <map>

struct KegItem
{
    std::string topicName;
    std::string formatStr;
    std::vector<std::vector<uint8_t>> data;
};

class Keg
{
public:
    Keg(const std::string& baseDir_in);
    void addItem(const std::string& name, const std::string& format);

private:
    std::map<std::string, KegItem> items;

    std::string baseDir;
};

#endif
