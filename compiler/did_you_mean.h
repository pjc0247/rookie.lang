#pragma once

#include <string>
#include <algorithm>

#include "runner.h"

struct id_candidate {
    std::wstring id;
    float distance;
};

class did_you_mean {
public:
    static std::vector<id_candidate> find(const std::wstring &id, runtime_typedata &type) {
        std::vector<id_candidate> result;

        for (auto &func : type.vtable) {
            auto sighash = func.first;
            auto candidate_id = rkctx()->get_name(sighash);
            auto dist = jaro(id, candidate_id);

            if (dist >= 0.7f) {
                id_candidate c;
                c.id = candidate_id;
                c.distance = dist;
                result.push_back(c);
            }
        }

        sort(result.begin(), result.end(), [](id_candidate &a, id_candidate &b) {
            return a.distance > b.distance;
        });

        return result;
    }

    static double jaro(const std::wstring &s1, const std::wstring &s2) {
        const int l1 = s1.length(), l2 = s2.length();
        if (l1 == 0)
            return l2 == 0 ? 1.0 : 0.0;

        const int match_distance = std::max(l1, l2) / 2 - 1;
        bool *s1_matches = new bool[l1];
        bool *s2_matches = new bool[l2];
        std::fill(s1_matches, s1_matches + l1, false);
        std::fill(s2_matches, s2_matches + l2, false);
        uint32_t matches = 0;
        for (int i = 0; i < l1; i++)
        {
            const int end = std::min(i + match_distance + 1, l2);
            for (int k = std::max(0, i - match_distance); k < end; k++)
                if (!s2_matches[k] && s1[i] == s2[k])
                {
                    s1_matches[i] = true;
                    s2_matches[k] = true;
                    matches++;
                    break;
                }
        }
        if (matches == 0) {
            delete[] s1_matches;
            delete[] s2_matches;
            return 0.0;
        }
        double t = 0.0;
        uint32_t k = 0;
        for (uint32_t i = 0; i < l1; i++)
            if (s1_matches[i])
            {
                while (!s2_matches[k]) k++;
                if (s1[i] != s2[k]) t += 0.5;
                k++;
            }

        const double m = matches;

        delete[] s1_matches;
        delete[] s2_matches;

        return (m / l1 + m / l2 + (m - t) / m) / 3.0;
    }
};