#ifndef INCLUDE_DATE_TO_STRING_HPP_
#define INCLUDE_DATE_TO_STRING_HPP_

#include <cstdint>
#include <string>

std::string date_to_string(uint16_t date, uint16_t time);

std::string date_to_string(uint32_t date);

#endif // INCLUDE_DATE_TO_STRING_HPP_
