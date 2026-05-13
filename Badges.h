#ifndef BADGES_H
#define BADGES_H

#include <string>
#include <unordered_map>
using namespace std;

struct BadgeStatistics
{
    int bronze = 0;
    int silver = 0;
    int gold   = 0;

    int total() const { return bronze + silver + gold; }
};

unordered_map<string, BadgeStatistics> badgeMap = {
    {"big_deck",    {}},
    {"elite",       {}},
    {"glutton",     {}},
    {"healer",      {}},
    {"highlander",  {}},
    {"honed",       {}},
    {"ilikeshiny",  {}},
    {"kaching",     {}},
    {"money_money", {}},
    {"perfect",     {}},
    {"restful",     {}},
    {"restless",    {}},
    {"speedy",      {}},
    {"team_player", {}},
    {"tiny_deck",   {}},
};

#endif
