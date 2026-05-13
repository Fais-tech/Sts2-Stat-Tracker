#ifndef character_h
#define character_h

#include <string>
#include <unordered_map>
using namespace std;

struct Character
{
    int totalRuns = 0;
    int wins      = 0;
    int abandoned = 0;

    float getWinRate() const
    {
        if (totalRuns == 0) return 0.0f;
        return (float)wins / totalRuns * 100.0f;
    }
};

unordered_map<string, Character> characterMap = {
    {"ironclad",   {}},
    {"silent",     {}},
    {"regent",     {}},
    {"necrobinder",{}},
    {"defect",     {}},
};

#endif
