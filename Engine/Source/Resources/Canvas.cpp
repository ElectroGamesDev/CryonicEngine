#include "Canvas.h"

std::unordered_map<std::string, nlohmann::json> Canvas::canvases;
Event Canvas::onDataChangeEvent;