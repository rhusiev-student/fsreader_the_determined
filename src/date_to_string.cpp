#include "date_to_string.hpp"
#include <cstdint>
#include <cstring>
#include <string>

std::string date_to_string(uint16_t date, uint16_t time) {
    std::string day = std::to_string(date & 0x1F);
    std::string month = std::to_string((date >> 5) & 0x0F);
    std::string year = std::to_string(1980 + ((date >> 9) & 0x7F));
    std::string hours = std::to_string((time >> 11) & 0x1F);
    std::string minutes = std::to_string((time >> 5) & 0x3F);
    std::string seconds = std::to_string(2 * (time & 0x1F));
    return year + '-' + (month.size() == 1 ? "0" + month : month) + '-' +
           (day.size() == 1 ? "0" + day : day) + ' ' +
           (hours.size() == 1 ? "0" + hours : hours) + ':' +
           (minutes.size() == 1 ? "0" + minutes : minutes) + ':' +
           (seconds.size() == 1 ? "0" + seconds : seconds);
}

int days_in_months[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

std::string date_to_string(uint32_t seconds) {
    std::string final = "";
    int days = seconds / 86400;
    seconds -= days * 86400;
    int year = 1970;

    while (true) {
        if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0)) {
            if (days < 366) {
                break;
            }
            days -= 366;
        } else {
            if (days < 365) {
                break;
            }
            days -= 365;
        }
        year++;
    }
    days++;
    bool current_leap = year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
    if (current_leap) {
        days_in_months[1] = 29;
    }

    for (int i = 0; i < 12; i++) {
        if (days <= days_in_months[i]) {
            final += std::to_string(year) + "-" +
                     (i + 1 < 10 ? "0" + std::to_string(i + 1)
                                 : std::to_string(i + 1)) +
                     "-" +
                     (days < 10 ? "0" + std::to_string(days)
                                : std::to_string(days)) +
                     " ";
            break;
        }
        days -= days_in_months[i];
    }

    int hours = seconds / 3600;
    seconds -= hours * 3600;
    int minutes = seconds / 60;
    seconds -= minutes * 60;
    final +=
        (hours < 10 ? "0" + std::to_string(hours) : std::to_string(hours)) +
        ":" +
        (minutes < 10 ? "0" + std::to_string(minutes)
                      : std::to_string(minutes)) +
        ":" +
        (seconds < 10 ? "0" + std::to_string(seconds)
                      : std::to_string(seconds));
    return final;
}
