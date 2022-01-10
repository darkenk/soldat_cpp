#pragma once

#include "common/Vector.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

struct tsoundsample
{
    bool loaded;
    std::uint32_t buffer;
};

struct tscriptsound
{
    std::string name;
    tsoundsample samp;
};

constexpr std::int32_t max_sources = 256;
constexpr std::int32_t reserved_sources = 128;
constexpr std::int32_t max_samples = 163;
constexpr std::int32_t channel_weather = 127;

extern std::vector<tscriptsound> scriptsamp;
extern float volumeinternal;
extern std::int32_t defaultchannel;

bool initsound();
std::int8_t soundnametoid(const std::string &name);
tsoundsample loadsample(const std::string_view &name, tsoundsample samp);
float scalevolumesetting(std::uint8_t volumesetting);
void loadsounds(std::string moddir);
void closesound();
void fplaysound(std::int32_t samplenum, float listenerx, float listenery, float emitterx,
                float emittery, std::int32_t chan);
void playsound(std::int32_t sample);
void playsound(std::int32_t sample, std::int32_t channel);
void playsound(std::int32_t sample, const tvector2 &emitter);
void playsound(std::int32_t sample, const tvector2 &emitter, int32_t channel);
bool stopsound(std::int32_t channel);

bool setsoundpaused(std::int32_t channel, bool paused);

bool setvolume(std::int32_t channel, float volume);
