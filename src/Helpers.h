#ifndef _HELPERS_H
#define _HELPERS_H

#include <string>
#include <vector>

//helper functions
class Helpers
{
 public:
  static void tokenize(const std::string& str,
		std::vector<std::string>& tokens,
		       const std::string& delimiters = " \t");
  static size_t countChars(const std::string& str, char c);
  static float str2float(const std::string& str);
  static int str2int(const std::string& str);

  static void getGLErrors(std::string where="");
};
#endif //_HELPERS_H
