#ifndef BASE64_H
#define BASE64_H

#include <string>

std::string base64_encode(unsigned char* , unsigned int len);
std::string base64_decode(std::string const& s);

#endif
