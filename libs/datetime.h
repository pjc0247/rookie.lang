#pragma once

#include <ctime>
#include <chrono>

#include "binding.h"

class rkdatetime : public rkobject<rkdatetime> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"datetime");

        static_method(type, L"now", now);

        method(type, L"month", &rkdatetime::month);

        b.add_type(type);
    }

    static value now() {
        auto date_time = new rkdatetime();

        auto n = std::chrono::system_clock::now();
        time_t tt = std::chrono::system_clock::to_time_t(n);
        date_time->time = *localtime(&tt);
        
        return obj2rk(date_time, L"datetime");
    }

    value month() {
        return int2rk(time.tm_mon);
    }

private:
    tm time;
};