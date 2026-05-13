#ifndef MONSTER_H
#define MONSTER_H

#include <string>
#include <unordered_map>
using namespace std;

struct Encounter
{
    string act;        // underdocks, overgrowth, hive, glory
    string difficulty; // normal, elite, boss
};

struct EncounterStatistics : Encounter
{
    int timesSeen         = 0;
    int timesKilledPlayer = 0;
    int totalDamageDealt  = 0;
    int totalTurnsTaken   = 0;

    float getAverageDamage() const
    {
        if (timesSeen == 0) return 0.0f;
        return (float)totalDamageDealt / timesSeen;
    }

    float getAverageTurns() const
    {
        if (timesSeen == 0) return 0.0f;
        return (float)totalTurnsTaken / timesSeen;
    }

    float getDeathRate() const
    {
        if (timesSeen == 0) return 0.0f;
        return (float)timesKilledPlayer / timesSeen * 100.0f;
    }
};

unordered_map<string, EncounterStatistics> monsterMap = {
    // underdocks normal
    {"corpse_slugs_normal",             {"underdocks", "normal"}},
    {"corpse_slugs_weak",               {"underdocks", "normal"}},
    {"cultists_normal",                 {"underdocks", "normal"}},
    {"fossil_stalker_normal",           {"underdocks", "normal"}},
    {"gremlin_merc_normal",             {"underdocks", "normal"}},
    {"haunted_ship_normal",             {"underdocks", "normal"}},
    {"living_fog_normal",               {"underdocks", "normal"}},
    {"punch_construct_normal",          {"underdocks", "normal"}},
    {"punch_off_event_encounter",       {"underdocks", "normal"}},
    {"seapunk_normal",                  {"underdocks", "normal"}},
    {"seapunk_weak",                    {"underdocks", "normal"}},
    {"sewer_clam_normal",               {"underdocks", "normal"}},
    {"sludge_spinner_weak",             {"underdocks", "normal"}},
    {"toadpoles_normal",                {"underdocks", "normal"}},
    {"toadpoles_weak",                  {"underdocks", "normal"}},
    {"two_tailed_rats_normal",          {"underdocks", "normal"}},
    // underdocks elites
    {"phantasmal_gardeners_elite",      {"underdocks", "elite"}},
    {"skulking_colony_elite",           {"underdocks", "elite"}},
    {"terror_eel_elite",                {"underdocks", "elite"}},
    // underdocks bosses
    {"lagavulin_matriarch_boss",        {"underdocks", "boss"}},
    {"soul_fysh_boss",                  {"underdocks", "boss"}},
    {"waterfall_giant_boss",            {"underdocks", "boss"}},
    // overgrowth normal
    {"cubex_construct_normal",          {"overgrowth", "normal"}},
    {"dense_vegetation_event_encounter",{"overgrowth", "normal"}},
    {"flyconid_normal",                 {"overgrowth", "normal"}},
    {"fogmog_normal",                   {"overgrowth", "normal"}},
    {"fuzzy_wurm_crawler_weak",         {"overgrowth", "normal"}},
    {"inklets_normal",                  {"overgrowth", "normal"}},
    {"mawler_normal",                   {"overgrowth", "normal"}},
    {"nibbits_normal",                  {"overgrowth", "normal"}},
    {"nibbits_weak",                    {"overgrowth", "normal"}},
    {"overgrowth_crawlers",             {"overgrowth", "normal"}},
    {"ruby_raiders_normal",             {"overgrowth", "normal"}},
    {"shrinker_beetle_weak",            {"overgrowth", "normal"}},
    {"slimes_normal",                   {"overgrowth", "normal"}},
    {"slimes_weak",                     {"overgrowth", "normal"}},
    {"slithering_strangler_normal",     {"overgrowth", "normal"}},
    {"snapping_jaxfruit_normal",        {"overgrowth", "normal"}},
    {"vine_shambler_normal",            {"overgrowth", "normal"}},
    // overgrowth elites
    {"bygone_effigy_elite",             {"overgrowth", "elite"}},
    {"byrdonis_elite",                  {"overgrowth", "elite"}},
    {"phrog_parasite_elite",            {"overgrowth", "elite"}},
    // overgrowth bosses
    {"ceremonial_beast_boss",           {"overgrowth", "boss"}},
    {"the_kin_boss",                    {"overgrowth", "boss"}},
    {"vantom_boss",                     {"overgrowth", "boss"}},
    // hive normal
    {"bowlbugs_normal",                 {"hive", "normal"}},
    {"bowlbugs_weak",                   {"hive", "normal"}},
    {"chompers_normal",                 {"hive", "normal"}},
    {"exoskeletons_normal",             {"hive", "normal"}},
    {"exoskeletons_weak",               {"hive", "normal"}},
    {"hunter_killer_normal",            {"hive", "normal"}},
    {"louse_progenitor_normal",         {"hive", "normal"}},
    {"mysterious_knight_event_encounter",{"hive", "normal"}},
    {"mytes_normal",                    {"hive", "normal"}},
    {"ovicopter_normal",                {"hive", "normal"}},
    {"slumbering_beetle_normal",        {"hive", "normal"}},
    {"spiny_toad_normal",               {"hive", "normal"}},
    {"the_obscura_normal",              {"hive", "normal"}},
    {"thieving_hopper_weak",            {"hive", "normal"}},
    {"tunneler_normal",                 {"hive", "normal"}},
    {"tunneler_weak",                   {"hive", "normal"}},
    // hive elites
    {"decimillipede_elite",             {"hive", "elite"}},
    {"entomancer_elite",                {"hive", "elite"}},
    {"infested_prisms_elite",           {"hive", "elite"}},
    // hive bosses
    {"kaiser_crab_boss",                {"hive", "boss"}},
    {"knowledge_demon_boss",            {"hive", "boss"}},
    {"the_insatiable_boss",             {"hive", "boss"}},
    // glory normal
    {"axebots_normal",                  {"glory", "normal"}},
    {"battleworn_dummy_event_encounter",{"glory", "normal"}},
    {"construct_menagerie_normal",      {"glory", "normal"}},
    {"devoted_sculptor_weak",           {"glory", "normal"}},
    {"fabricator_normal",               {"glory", "normal"}},
    {"frog_knight_normal",              {"glory", "normal"}},
    {"globe_head_normal",               {"glory", "normal"}},
    {"owl_magistrate_normal",           {"glory", "normal"}},
    {"scrolls_of_biting_normal",        {"glory", "normal"}},
    {"scrolls_of_biting_weak",          {"glory", "normal"}},
    {"slimed_berserker_normal",         {"glory", "normal"}},
    {"the_lost_and_forgotten_normal",   {"glory", "normal"}},
    {"turret_operator_weak",            {"glory", "normal"}},
    // glory elites
    {"knights_elite",                   {"glory", "elite"}},
    {"mecha_knight_elite",              {"glory", "elite"}},
    {"soul_nexus_elite",                {"glory", "elite"}},
    // glory bosses
    {"doormaker_boss",                  {"glory", "boss"}},
    {"queen_boss",                      {"glory", "boss"}},
    {"test_subject_boss",               {"glory", "boss"}},
};

#endif