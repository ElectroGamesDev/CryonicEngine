#include "Particles.h"
std::unordered_map<std::string, nlohmann::json> Particles::particles;
Event Particles::onDataChangeEvent;