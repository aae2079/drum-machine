#ifndef DRUM_MACHINE_JSON_PARSER_H
#define DRUM_MACHINE_JSON_PARSER_H

#include <string>
#include <iostream>
#include "simDefs.hpp"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"


static bool parseJsonSettings(const std::string& filename, Params& params) {
    rapidjson::Document doc;
    if (!doc.Parse(filename.c_str()).HasParseError()) {
        if (doc.HasMember("audio") && doc["audio"].IsObject()) {
            const auto& audio = doc["audio"];
            params.audio.sampleRate = audio["sample_rate"].GetFloat();
            params.audio.bitDepth = audio["bit_depth"].GetInt();
            params.audio.numChannels = audio["num_channels"].GetInt();
            params.audio.audioFormatPCM = audio["audio_format_pcm"].GetInt();
            params.audio.byteRate = params.audio.sampleRate * params.audio.numChannels * params.audio.bitDepth / 8; 
            params.audio.blockAlign = params.audio.numChannels * params.audio.bitDepth / 8;
        }
        if (doc.HasMember("timbre") && doc["timbre"].IsObject()) {
            const auto& timbre = doc["timbre"];
            params.timbre.membrane_thickness = timbre["membrane_thickness"].GetFloat();
            params.timbre.material_density = params.timbre.membrane_thickness * timbre["material_density"].GetFloat();
            params.timbre.tension = timbre["tension"].GetFloat();
            params.timbre.radius = timbre["radius"].GetFloat();
            params.timbre.damping = timbre["damping"].GetFloat();
        }
        if (doc.HasMember("grid") && doc["grid"].IsObject()) {
            const auto& grid = doc["grid"];
            params.grid.grid_r = grid["grid_r"].GetUint();
            params.grid.grid_th = grid["grid_th"].GetUint();
        }
        return true;
    } else {
        std::cerr << "Error parsing JSON: " << rapidjson::GetParseError_En(doc.GetParseError()) << std::endl;
        return false;
    }
};

#endif