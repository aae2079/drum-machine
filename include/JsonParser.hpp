#ifndef DRUM_MACHINE_JSON_PARSER_H
#define DRUM_MACHINE_JSON_PARSER_H

#include <string>
#include "simDefs.hpp"
#include "rapidjson/document.h"

class JsonParser {
public:
    static bool parseJsonSettings(const std::string& filename, Params& params) {};
};
#endif