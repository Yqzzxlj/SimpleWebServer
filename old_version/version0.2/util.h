#include <sys/types.h>
#include <string>

namespace util {
// std::string trim functions

std::string& ltrim(std::string& str);

std::string& rtrim(std::string& str);

std::string& trim(std::string& str);

} // namespace util
