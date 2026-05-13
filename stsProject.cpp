#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>
#include <filesystem>
#include <unordered_set>
#include <iomanip>
#include "Cards.h"
#include "Relics.h"
#include "Monster.h"
#include "Character.h"
#include "Badges.h"
#include "nlohmann/json.hpp"
using namespace std;
namespace fs = std::filesystem;

const unordered_set<string> removedCards = {
    "grapple",
};

const unordered_set<string> characterNames = {
    "ironclad", "silent", "regent", "necrobinder", "defect",
};

string formatName(const string& name)
{
    string result = name;
    replace(result.begin(), result.end(), '_', ' ');
    bool capitalizeNext = true;
    for (char& c : result)
    {
        if (c == ' ') { capitalizeNext = true; }
        else if (capitalizeNext) { c = toupper(c); capitalizeNext = false; }
    }
    return result;
}

int main(int argc, char* argv[])
{
    string historyPath = (argc > 1) ? argv[1] : "history";
    int filesProcessed = 0;

    cout << "Parsing run files...\n";

    for (auto& entry : fs::directory_iterator(historyPath))
    {
        if (entry.path().extension() != ".run") continue;

        ifstream data(entry.path());
        if (!data.is_open())
        {
            cout << "Could not open: " << entry.path() << "\n";
            continue;
        }

        nlohmann::json run;
        try { run = nlohmann::json::parse(data); }
        catch (...) { cout << "Failed to parse: " << entry.path() << "\n"; continue; }

        filesProcessed++;
        // cout << "Processing: " << entry.path().filename().string() << "\n";

        bool runWon       = run["win"];
        bool runAbandoned = run["was_abandoned"];

        string charId = run["players"][0]["character"];
        string charName = charId.substr(10);
        transform(charName.begin(), charName.end(), charName.begin(), ::tolower);

        if (characterMap.count(charName))
        {
            characterMap[charName].totalRuns++;
            if (runWon)       characterMap[charName].wins++;
            if (runAbandoned) characterMap[charName].abandoned++;
        }
        else
        {
            cout << "UNMATCHED CHARACTER: " << charName << "\n";
        }

        for (auto& player : run["players"])
        {
            if (!player.contains("badges")) continue;
            for (auto& badge : player["badges"])
            {
                string badgeId = badge["id"];
                string badgeName = badgeId;
                transform(badgeName.begin(), badgeName.end(), badgeName.begin(), ::tolower);
                string rarity = badge["rarity"];

                if (badgeMap.count(badgeName))
                {
                    if      (rarity == "bronze") badgeMap[badgeName].bronze++;
                    else if (rarity == "silver") badgeMap[badgeName].silver++;
                    else if (rarity == "gold")   badgeMap[badgeName].gold++;
                }
                else
                {
                    cout << "UNMATCHED BADGE: " << badgeName << "\n";
                }
            }
        }

        for (auto& act : run["map_point_history"])
        {
            for (auto& mapPoint : act)
            {
                // Parse card choices
                for (auto& playerStat : mapPoint["player_stats"])
                {
                    if (playerStat.contains("card_choices"))
                    {
                        for (auto& choice : playerStat["card_choices"])
                        {
                            string cardId = choice["card"]["id"];
                            string cardName = cardId.substr(5);
                            transform(cardName.begin(), cardName.end(), cardName.begin(), ::tolower);

                            bool wasPicked = choice["was_picked"];

                            if (removedCards.count(cardName))
                            {
                                // silently skip removed cards
                            }
                            else if (cardMap.count(cardName))
                            {
                                cardMap[cardName].timesSeen++;
                                if (wasPicked)
                                {
                                    cardMap[cardName].timesPicked++;
                                    if (runWon)
                                        cardMap[cardName].timesWon++;
                                }
                            }
                            else
                            {
                                cout << "UNMATCHED CARD: " << cardName << "\n";
                            }
                        }
                    }

                    if (playerStat.contains("cards_gained"))
                    {
                        for (auto& gained : playerStat["cards_gained"])
                        {
                            string cardId = gained["id"];
                            string cardName = cardId.substr(5);
                            transform(cardName.begin(), cardName.end(), cardName.begin(), ::tolower);

                            if (cardMap.count(cardName))
                            {
                                const string& rarity = cardMap[cardName].cardRarity;
                                if (rarity == "curse" || rarity == "quest")
                                {
                                    cardMap[cardName].timesSeen++;
                                    if (runWon)
                                        cardMap[cardName].timesWon++;
                                }
                            }
                        }
                    }

                    if (playerStat.contains("relic_choices"))
                    {
                        for (auto& rc : playerStat["relic_choices"])
                        {
                            string relicId = rc["choice"];
                            string relicName = relicId.substr(6);
                            transform(relicName.begin(), relicName.end(), relicName.begin(), ::tolower);

                            bool wasPicked = rc["was_picked"];

                            if (relicMap.count(relicName))
                            {
                                relicMap[relicName].timesSeen++;
                                if (wasPicked)
                                {
                                    relicMap[relicName].timesPicked++;
                                    if (runWon)
                                        relicMap[relicName].timesWon++;
                                }
                            }
                            else
                            {
                                cout << "UNMATCHED RELIC: " << relicName << "\n";
                            }
                        }
                    }

                    if (playerStat.contains("ancient_choice"))
                    {
                        for (auto& ac : playerStat["ancient_choice"])
                        {
                            bool wasChosen = ac["was_chosen"];
                            if (wasChosen) continue; // already counted via relic_choices

                            string relicName = ac["TextKey"];
                            transform(relicName.begin(), relicName.end(), relicName.begin(), ::tolower);

                            if (characterNames.count(relicName)) continue; // starter relic upgrade choice, not a relic

                            if (relicMap.count(relicName))
                                relicMap[relicName].timesSeen++;
                            else
                                cout << "UNMATCHED RELIC (ancient): " << relicName << "\n";
                        }
                    }
                }

                // Parse encounter statistics
                for (auto& room : mapPoint["rooms"])
                {
                    if (!room.contains("model_id")) continue;
                    if (room["room_type"] != "monster" &&
                        room["room_type"] != "elite" &&
                        room["room_type"] != "boss") continue;

                    string encounterId = room["model_id"];
                    encounterId = encounterId.substr(10);
                    transform(encounterId.begin(), encounterId.end(), encounterId.begin(), ::tolower);

                    int turnsTaken = room["turns_taken"];

                    if (monsterMap.count(encounterId))
                    {
                        monsterMap[encounterId].timesSeen++;
                        monsterMap[encounterId].totalTurnsTaken += turnsTaken;
                    }
                    else
                    {
                        cout << "UNMATCHED ENCOUNTER: " << encounterId << "\n";
                    }

                    for (auto& playerStat : mapPoint["player_stats"])
                    {
                        if (!playerStat.contains("damage_taken")) continue;
                        int damageTaken = playerStat["damage_taken"];
                        if (monsterMap.count(encounterId))
                            monsterMap[encounterId].totalDamageDealt += damageTaken;
                    }
                }
            }
        }

        // Check if killed by encounter
        string killedBy = run["killed_by_encounter"];
        if (killedBy != "NONE.NONE")
        {
            killedBy = killedBy.substr(10);
            transform(killedBy.begin(), killedBy.end(), killedBy.begin(), ::tolower);
            if (monsterMap.count(killedBy))
                monsterMap[killedBy].timesKilledPlayer++;
        }
    }

    cout << "\nProcessed " << filesProcessed << " run file(s)." << "\n";

    // ------------------------------------------------
    // Character Statistics
    // ------------------------------------------------
    auto charOrder = [](const string& cls) -> int
    {
        if (cls == "ironclad")    return 0;
        if (cls == "silent")      return 1;
        if (cls == "regent")      return 2;
        if (cls == "necrobinder") return 3;
        if (cls == "defect")      return 4;
        return 5;
    };

    vector<pair<string, Character*>> playedCharacters;
    for (auto& [name, stats] : characterMap)
        if (stats.totalRuns > 0)
            playedCharacters.push_back({name, &stats});

    sort(playedCharacters.begin(), playedCharacters.end(), [&charOrder](auto& a, auto& b)
    {
        return charOrder(a.first) < charOrder(b.first);
    });

    cout << fixed << setprecision(2);

    size_t maxCharLen = 0;
    for (auto& [name, _] : playedCharacters)
        maxCharLen = max(maxCharLen, formatName(name).size());

    int totalRuns = 0, totalWins = 0, totalAbandoned = 0;
    for (auto& [name, stats] : playedCharacters)
    {
        totalRuns      += stats->totalRuns;
        totalWins      += stats->wins;
        totalAbandoned += stats->abandoned;
    }
    float overallWinRate = totalRuns > 0 ? (float)totalWins / totalRuns * 100.0f : 0.0f;

    size_t overallLabelLen = max(maxCharLen, string("Overall").size());

    cout << "\n========== Character Statistics ==========" << "\n";
    for (auto& [name, stats] : playedCharacters)
    {
        cout << "  " << left << setw(overallLabelLen) << formatName(name) << right
             << " | Runs: "      << setw(3) << stats->totalRuns
             << " | Wins: "      << setw(3) << stats->wins
             << " | Win Rate: "  << setw(6) << stats->getWinRate() << "%"
             << " | Abandoned: " << setw(3) << stats->abandoned
             << "\n";
    }
    cout << "\n  " << left << setw(overallLabelLen) << "Overall" << right
         << " | Runs: "      << setw(3) << totalRuns
         << " | Wins: "      << setw(3) << totalWins
         << " | Win Rate: "  << setw(6) << overallWinRate << "%"
         << " | Abandoned: " << setw(3) << totalAbandoned
         << "\n";

    // ------------------------------------------------
    // Badge Statistics
    // ------------------------------------------------
    vector<pair<string, BadgeStatistics*>> earnedBadges;
    for (auto& [name, stats] : badgeMap)
        if (stats.total() > 0)
            earnedBadges.push_back({name, &stats});

    sort(earnedBadges.begin(), earnedBadges.end(), [](auto& a, auto& b)
    {
        return a.second->total() > b.second->total();
    });

    size_t maxBadgeLen = 0;
    for (auto& [name, _] : earnedBadges)
        maxBadgeLen = max(maxBadgeLen, formatName(name).size());

    cout << "\n========== Badge Statistics ==========" << "\n";
    for (auto& [name, stats] : earnedBadges)
    {
        cout << "  " << left << setw(maxBadgeLen) << formatName(name) << right
             << " | Total: " << setw(3) << stats->total()
             << " | Bronze: " << setw(3) << stats->bronze
             << " | Silver: " << setw(3) << stats->silver
             << " | Gold: "   << setw(3) << stats->gold
             << "\n";
    }

    // ------------------------------------------------
    // Relic Statistics
    // ------------------------------------------------
    vector<pair<string, RelicStatistics*>> seenRelics;
    for (auto& [name, stats] : relicMap)
        if (stats.timesSeen > 0)
            seenRelics.push_back({name, &stats});

    auto relicRarityOrder = [](const string& rarity) -> int
    {
        if (rarity == "common")   return 0;
        if (rarity == "uncommon") return 1;
        if (rarity == "rare")     return 2;
        if (rarity == "ancient")  return 3;
        if (rarity == "shop")     return 4;
        if (rarity == "event")    return 5;
        return 6;
    };

    sort(seenRelics.begin(), seenRelics.end(), [&relicRarityOrder](auto& a, auto& b)
    {
        if (a.second->relicRarity != b.second->relicRarity)
            return relicRarityOrder(a.second->relicRarity) < relicRarityOrder(b.second->relicRarity);
        return a.first < b.first;
    });

    size_t maxRelicLen = 0;
    for (auto& [name, _] : seenRelics)
        maxRelicLen = max(maxRelicLen, formatName(name).size());

    string currentRelicRarity = "NONE";
    cout << "\n========== Relic Statistics ==========" << "\n";
    for (auto& [name, stats] : seenRelics)
    {
        if (stats->relicRarity != currentRelicRarity)
        {
            currentRelicRarity = stats->relicRarity;
            cout << "\n  -- " << formatName(currentRelicRarity) << " --" << "\n";
        }
        cout << "  " << left << setw(maxRelicLen) << formatName(name) << right
             << " | Pick Rate: " << setw(6) << stats->getPickRate() << "%"
             << " | Picked: "    << setw(3) << stats->timesPicked
             << "/"              << setw(3) << stats->timesSeen
             << " | Win Rate: "  << setw(6) << stats->getWinRate() << "%"
             << " | Wins: "      << setw(3) << stats->timesWon
             << "\n";
    }

    // ------------------------------------------------
    // Card Statistics
    // ------------------------------------------------
    vector<pair<string, CardStatistics*>> seenCards;
    for (auto& [name, stats] : cardMap)
    {
        if (stats.timesSeen > 0)
            seenCards.push_back({name, &stats});
    }

    auto classOrder = [](const string& cls) -> int
    {
        if (cls == "ironclad")    return 0;
        if (cls == "silent")      return 1;
        if (cls == "regent")      return 2;
        if (cls == "necrobinder") return 3;
        if (cls == "defect")      return 4;
        if (cls == "colorless")   return 5;
        return 6;
    };

    auto rarityOrder = [](const string& rarity) -> int
    {
        if (rarity == "basic")    return 0;
        if (rarity == "common")   return 1;
        if (rarity == "uncommon") return 2;
        if (rarity == "rare")     return 3;
        if (rarity == "ancient")  return 4;
        if (rarity == "event")    return 5;
        if (rarity == "status")   return 6;
        if (rarity == "quest")    return 7;
        if (rarity == "curse")    return 8;
        return 9;
    };

    sort(seenCards.begin(), seenCards.end(), [&classOrder, &rarityOrder](auto& a, auto& b)
    {
        if (a.second->cardClass != b.second->cardClass)
            return classOrder(a.second->cardClass) < classOrder(b.second->cardClass);
        if (a.second->cardRarity != b.second->cardRarity)
            return rarityOrder(a.second->cardRarity) < rarityOrder(b.second->cardRarity);
        return a.first < b.first;
    });

    size_t maxCardLen = 0;
    for (auto& [name, _] : seenCards)
        maxCardLen = max(maxCardLen, formatName(name).size());

    string currentClass = "NONE";
    string currentRarity = "NONE";
    cout << "\n========== Card Statistics ==========" << "\n";
    for (auto& [name, stats] : seenCards)
    {
        if (stats->cardClass != currentClass)
        {
            currentClass = stats->cardClass;
            currentRarity = "NONE";
            cout << "\n=== " << formatName(currentClass) << " ===" << "\n";
        }
        if (stats->cardRarity != currentRarity)
        {
            currentRarity = stats->cardRarity;
            cout << "\n  -- " << formatName(currentRarity) << " --" << "\n";
        }
        if (stats->cardRarity == "curse" || stats->cardRarity == "quest")
        {
            cout << "  " << left << setw(maxCardLen) << formatName(name) << right
                 << " | Received: " << setw(3) << stats->timesSeen
                 << " | Wins: "     << setw(3) << stats->timesWon << "\n";
        }
        else
        {
            cout << "  " << left << setw(maxCardLen) << formatName(name) << right
                 << " | Pick Rate: " << setw(6) << stats->getPickRate() << "%"
                 << " | Picked: "    << setw(3) << stats->timesPicked
                 << "/"              << setw(3) << stats->timesSeen
                 << " | Win Rate: "  << setw(6) << stats->getWinRate() << "%"
                 << " | Wins: "      << setw(3) << stats->timesWon << "\n";
        }
    }

    // ------------------------------------------------
    // Encounter Statistics
    // ------------------------------------------------
    vector<pair<string, EncounterStatistics*>> seenMonsters;
    for (auto& [name, stats] : monsterMap)
    {
        if (stats.timesSeen > 0)
            seenMonsters.push_back({name, &stats});
    }

    auto difficultyOrder = [](const string& difficulty) -> int
    {
        if (difficulty == "normal") return 0;
        if (difficulty == "elite")  return 1;
        if (difficulty == "boss")   return 2;
        return 3;
    };

    sort(seenMonsters.begin(), seenMonsters.end(), [&difficultyOrder](auto& a, auto& b)
    {
        if (a.second->act != b.second->act)
            return a.second->act > b.second->act;
        if (a.second->difficulty != b.second->difficulty)
            return difficultyOrder(a.second->difficulty) < difficultyOrder(b.second->difficulty);
        return a.first < b.first;
    });

    size_t maxMonsterLen = 0;
    for (auto& [name, _] : seenMonsters)
        maxMonsterLen = max(maxMonsterLen, formatName(name).size());

    string currentAct = "NONE";
    string currentDifficulty = "NONE";
    cout << "\n========== Monster Statistics ==========" << "\n";
    for (auto& [name, stats] : seenMonsters)
    {
        if (stats->act != currentAct)
        {
            currentAct = stats->act;
            currentDifficulty = "NONE";
            cout << "\n=== " << formatName(currentAct) << " ===" << "\n";
        }
        if (stats->difficulty != currentDifficulty)
        {
            currentDifficulty = stats->difficulty;
            cout << "\n  -- " << formatName(currentDifficulty) << " --" << "\n";
        }
        cout << "  " << left << setw(maxMonsterLen) << formatName(name) << right
             << " | Seen: "          << setw(3) << stats->timesSeen
             << " | Avg Damage: "    << setw(6) << stats->getAverageDamage()
             << " | Avg Turns: "     << setw(5) << stats->getAverageTurns()
             << " | Killed Player: " << setw(3) << stats->timesKilledPlayer
             << "\n";
    }

    // ------------------------------------------------
    // CSV Output
    // ------------------------------------------------
    fs::create_directories("output");

    {
        ofstream csv("output/characters.csv");
        csv << "Character,Runs,Wins,Win Rate,Abandoned\n";
        for (auto& [name, stats] : playedCharacters)
            csv << formatName(name) << "," << stats->totalRuns << "," << stats->wins << ","
                << fixed << setprecision(2) << stats->getWinRate() << "," << stats->abandoned << "\n";
        csv << "Overall," << totalRuns << "," << totalWins << ","
            << fixed << setprecision(2) << overallWinRate << "," << totalAbandoned << "\n";
    }

    {
        ofstream csv("output/badges.csv");
        csv << "Badge,Total,Bronze,Silver,Gold\n";
        for (auto& [name, stats] : earnedBadges)
            csv << formatName(name) << "," << stats->total() << "," << stats->bronze << ","
                << stats->silver << "," << stats->gold << "\n";
    }

    {
        ofstream csv("output/relics.csv");
        csv << "Relic,Rarity,Times Seen,Times Picked,Pick Rate,Wins,Win Rate\n";
        for (auto& [name, stats] : seenRelics)
            csv << formatName(name) << "," << stats->relicRarity << "," << stats->timesSeen << ","
                << stats->timesPicked << "," << fixed << setprecision(2) << stats->getPickRate() << ","
                << stats->timesWon << "," << stats->getWinRate() << "\n";
    }

    {
        ofstream csv("output/cards.csv");
        csv << "Card,Class,Rarity,Times Seen,Times Picked,Pick Rate,Wins,Win Rate\n";
        for (auto& [name, stats] : seenCards)
        {
            if (stats->cardRarity == "curse" || stats->cardRarity == "quest")
                csv << formatName(name) << "," << stats->cardClass << "," << stats->cardRarity << ","
                    << stats->timesSeen << ",,,," << stats->timesWon << "\n";
            else
                csv << formatName(name) << "," << stats->cardClass << "," << stats->cardRarity << ","
                    << stats->timesSeen << "," << stats->timesPicked << ","
                    << fixed << setprecision(2) << stats->getPickRate() << ","
                    << stats->timesWon << "," << stats->getWinRate() << "\n";
        }
    }

    {
        ofstream csv("output/encounters.csv");
        csv << "Encounter,Act,Difficulty,Times Seen,Avg Damage,Avg Turns,Times Killed Player\n";
        for (auto& [name, stats] : seenMonsters)
            csv << formatName(name) << "," << stats->act << "," << stats->difficulty << ","
                << stats->timesSeen << "," << fixed << setprecision(2) << stats->getAverageDamage() << ","
                << stats->getAverageTurns() << "," << stats->timesKilledPlayer << "\n";
    }

    cout << "\nCSV files written to output/" << "\n";

    return 0;
}