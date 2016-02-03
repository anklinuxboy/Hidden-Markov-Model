#ifndef GUARD_UTILS_HPP
#define GUARD_UTILS_HPP

#include <string>
#include <vector>

/** Return a vector of this line split into space delimited words. */
template <typename T> std::vector<T> split(const std::string& line);
/** Return vector of observation sequences in an .obs file. */
std::vector<std::vector<std::string> > parseObsFile(const std::string& filename);


#endif
