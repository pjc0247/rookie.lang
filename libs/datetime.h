#pragma once

#include <ctime>
#include <chrono>

#include "binding.h"

class rkdatetime : public rkobject<rkdatetime> {
public:
    TYPENAME(L"datetime")

    static void _import(binding &b) {
        auto type = type_builder(L"datetime");

        static_method(type, L"now", now);

        method(type, L"year", &rkdatetime::year);
        method(type, L"month", &rkdatetime::month);
        method(type, L"day", &rkdatetime::day);
        method(type, L"hour", &rkdatetime::hour);
        method(type, L"minute", &rkdatetime::hour);
        method(type, L"second", &rkdatetime::second);

        b.add_type(type);
    }

    static value now() {
        auto date_time = new rkdatetime();

        auto n = std::chrono::system_clock::now();
        time_t tt = std::chrono::system_clock::to_time_t(n);
        date_time->time = *localtime(&tt);
        
        return obj2rk(date_time);
    }

    value year() {
        return int2rk(time.tm_year);
    }
    value month() {
        return int2rk(time.tm_mon);
    }
    value day() {
        return int2rk(time.tm_mday);
    }
    value hour() {
        return int2rk(time.tm_hour);
    }
    value minute() {
        return int2rk(time.tm_min);
    }
    value second() {
        return int2rk(time.tm_sec);
    }

private:
    tm time;
};