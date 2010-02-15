#ifndef _HELPERS_H
#define _HELPERS_H

#include <string>
#include <vector>

//helper functions
class Helpers
{
 public:
  static void Tokenize(const std::string& str,
		std::vector<std::string>& tokens,
		const std::string& delimiters = " ");
};
#endif //_HELPERS_H
