#ifndef DRUM_MACHINE_JSON_PARSER_H
#define DRUM_MACHINE_JSON_PARSER_H

#include <string>
#include <iostream>
#include "simDefs.hpp"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/filereadstream.h"
#include <cstdio>


static bool parseJsonSettings(const std::string& filename, Params& params) {
    FILE *fp = fopen(filename.c_str(), "r");
    if (!fp){
        std::cerr << "Could not open file: " << filename << std::endl;
        return false;
    }
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidjson::Document doc;
    doc.ParseStream(is);
    fclose(fp);

    if (!doc.HasParseError()) {
        if (doc.HasMember("audio") && doc["audio"].IsObject()) {
            const auto& audio = doc["audio"];
            params.audio.sampleRate = audio["sample_rate"].GetFloat();
            params.audio.bufferSize = audio["buffer_size"].GetInt();
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
            params.timbre.shell_length = timbre["shell_length"].GetFloat();
            params.timbre.damping = timbre["damping"].GetFloat();
        }
        if (doc.HasMember("dimensions") && doc["dimensions"].IsObject()) {
            const auto& dimensions = doc["dimensions"];
            params.grid.grid_r = dimensions["grid_r"].GetUint();
            params.grid.grid_th = dimensions["grid_th"].GetUint();
        }
        if (doc.HasMember("zoom") && doc["zoom"].IsObject()) {
            const auto& zoom = doc["zoom"];
            params.zoom_sensitivity = zoom["sensitivity_constant"].GetFloat();
        }
        return true;
    } else {
        std::cerr << "Error parsing JSON: " << rapidjson::GetParseError_En(doc.GetParseError()) << std::endl;
        return false;
    }
};

#endif