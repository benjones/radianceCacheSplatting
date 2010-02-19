#include "Helpers.h"
#include <sstream>
//from http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
void Helpers::tokenize(const std::string& str,
		       std::vector<std::string>& tokens,
		       const std::string& delimiters)
{
  // Skip delimiters at beginning.
  std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
  std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

  while (std::string::npos != pos || std::string::npos != lastPos)
    {
      // Found a token, add it to the vector.
      tokens.push_back(str.substr(lastPos, pos - lastPos));
      // Skip delimiters.  Note the "not_of"
      lastPos = str.find_first_not_of(delimiters, pos);
      // Find next "non-delimiter"
      pos = str.find_first_of(delimiters, lastPos);
    }
}

size_t Helpers::countChars(const std::string& str, char c)
{
  size_t count = 0;
  std::string::size_type found = str.find_first_of(c);
  while(found != std::string::npos)
    {
      ++count;
      found = str.find_first_of(c, found+1);
    }

  return count;
}

float Helpers::str2float(const std::string& str)
{
  std::istringstream stream(str);
  float f;
  stream >> f;
  return f;
}
int Helpers::str2int(const std::string& str)
{
  std::istringstream stream(str);
  int i;
  stream >> i;
  return i;
}
