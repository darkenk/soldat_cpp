// automatically converted

#include "Sound.hpp"
#include "Client.hpp"
#include "common/Logging.hpp"
#include "common/Parts.hpp"
#include "common/PhysFSExt.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/Constants.hpp"
#include "shared/Cvar.hpp"
#include <SDL2/SDL.h>
#include <array>
#include <soloud.h>
#include <soloud_wav.h>
#include <string>

// clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

using string = std::string;

PascalArray<tsoundsample, 1, max_samples> samp;
std::vector<tscriptsound> scriptsamp;
float volumeinternal = 0.5f;
std::int32_t defaultchannel = -1;

static std::string_view AUDIO = "audio";

[[deprecated("indexing from 1")]] const std::array<string, max_samples + 1> sample_files = {
    {"",
     "ak74-fire.wav",
     "rocketz.wav",
     "ak74-reload.wav",
     "", // empty.wav - no longer used
     "m249-fire.wav",
     "ruger77-fire.wav",
     "ruger77-reload.wav",
     "m249-reload.wav",
     "mp5-fire.wav",
     "mp5-reload.wav",
     "spas12-fire.wav",
     "spas12-reload.wav",
     "standup.wav",
     "fall.wav",
     "spawn.wav",
     "m79-fire.wav",
     "m79-explosion.wav",
     "m79-reload.wav",
     "grenade-throw.wav",
     "grenade-explosion.wav",
     "grenade-bounce.wav",
     "bryzg.wav",
     "infiltmus.wav",
     "headchop.wav",
     "explosion-erg.wav",
     "water-step.wav",
     "bulletby.wav",
     "bodyfall.wav",
     "deserteagle-fire.wav",
     "deserteagle-reload.wav",
     "steyraug-fire.wav",
     "steyraug-reload.wav",
     "barretm82-fire.wav",
     "barretm82-reload.wav",
     "minigun-fire.wav",
     "minigun-reload.wav",
     "minigun-start.wav",
     "minigun-end.wav",
     "pickupgun.wav",
     "capture.wav",
     "colt1911-fire.wav",
     "colt1911-reload.wav",
     "changeweapon.wav",
     "shell.wav",
     "shell2.wav",
     "dead-hit.wav",
     "throwgun.wav",
     "bow-fire.wav",
     "takebow.wav",
     "takemedikit.wav",
     "wermusic.wav",
     "ts.wav",
     "ctf.wav",
     "berserker.wav",
     "godflame.wav",
     "flamer.wav",
     "predator.wav",
     "killberserk.wav",
     "vesthit.wav",
     "burn.wav",
     "vesttake.wav",
     "clustergrenade.wav",
     "cluster-explosion.wav",
     "grenade-pullout.wav",
     "spit.wav",
     "stuff.wav",
     "smoke.wav",
     "match.wav",
     "roar.wav",
     "step.wav",
     "step2.wav",
     "step3.wav",
     "step4.wav",
     "hum.wav",
     "ric.wav",
     "ric2.wav",
     "ric3.wav",
     "ric4.wav",
     "dist-m79.wav",
     "dist-grenade.wav",
     "dist-gun1.wav",
     "dist-gun2.wav",
     "dist-gun3.wav",
     "dist-gun4.wav",
     "death.wav",
     "death2.wav",
     "death3.wav",
     "crouch-move.wav",
     "hit-arg.wav",
     "hit-arg2.wav",
     "hit-arg3.wav",
     "goprone.wav",
     "roll.wav",
     "fall-hard.wav",
     "onfire.wav",
     "firecrack.wav",
     "scope.wav",
     "scopeback.wav",
     "playerdeath.wav",
     "changespin.wav",
     "arg.wav",
     "lava.wav",
     "regenerate.wav",
     "prone-move.wav",
     "jump.wav",
     "crouch.wav",
     "crouch-movel.wav",
     "step5.wav",
     "step6.wav",
     "step7.wav",
     "step8.wav",
     "stop.wav",
     "bulletby2.wav",
     "bulletby3.wav",
     "bulletby4.wav",
     "bulletby5.wav",
     "weaponhit.wav",
     "clipfall.wav",
     "bonecrack.wav",
     "gaugeshell.wav",
     "colliderhit.wav",
     "kit-fall.wav",
     "kit-fall2.wav",
     "flag.wav",
     "flag2.wav",
     "takegun.wav",
     "infilt-point.wav",
     "menuclick.wav",
     "knife.wav",
     "slash.wav",
     "chainsaw-d.wav",
     "chainsaw-m.wav",
     "chainsaw-r.wav",
     "piss.wav",
     "law.wav",
     "chainsaw-o.wav",
     "m2fire.wav",
     "m2explode.wav",
     "m2overheat.wav",
     "signal.wav",
     "m2use.wav",
     "scoperun.wav",
     "mercy.wav",
     "ric5.wav",
     "ric6.wav",
     "ric7.wav",
     "law-start.wav",
     "law-end.wav",
     "boomheadshot.wav",
     "snapshot.wav",
     "radio/efcup.wav",
     "radio/efcmid.wav",
     "radio/efcdown.wav",
     "radio/ffcup.wav",
     "radio/ffcmid.wav",
     "radio/ffcdown.wav",
     "radio/esup.wav",
     "radio/esmid.wav",
     "radio/esdown.wav",
     "bounce.wav",
     "sfx_rain.wav",
     "sfx_snow.wav",
     "sfx_wind.wav"}};

class SoundEngine
{
  public:
    bool InitSound()
    {
        LogDebug(AUDIO, "InitSound");
        if (Engine.init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::SDL2) != 0)
        {
            return false;
        }
        for (auto i = 1; i <= max_samples; i++)
        {
            samp[i].loaded = false;
            samp[i].buffer = i;
            Waves[i] = nullptr;
        }
        Engine.setGlobalVolume(volumeinternal);
        return true;
    }

    tsoundsample LoadSample(const std::string_view &name, tsoundsample samp)
    {
        PhysFS_Buffer filebuffer;

        tsoundsample result;
        if (!samp.loaded)
            result.loaded = false;
        else
            result = samp;

        filebuffer = PhysFS_readBuffer(name);
        if (filebuffer.empty())
        {
            LogWarn(AUDIO, "Empty audio {}", name);
            return result;
        }

        SoLoud::Wav *wav = new SoLoud::Wav();
        auto wavError = wav->loadMem(filebuffer.data(), length(filebuffer), true);
        if (wavError != 0)
        {
            delete wav;
            LogWarn(AUDIO, "Cannot create wav{}", wavError);
            return result;
        }

        SoldatAssert(samp.loaded == false);
        Waves[samp.buffer] = wav;

        result.loaded = true;
        return result;
    }

    void CloseSound()
    {
        for (auto i = Waves.StartIdx(); i <= Waves.EndIdx(); i++)
        {
            delete Waves[i];
        }
        Engine.deinit();
    }

    void fplaysound(std::int32_t samplenum, float listenerx, float listenery, float emitterx,
                    float emittery, std::int32_t chan)
    {
        float dist;
        std::int32_t looping;
        std::int32_t i;
        // Pan: Single = 0.0;

        if (!samp[samplenum].loaded)
            return;

        if (camerafollowsprite > 0)
        {
            listenerx = spriteparts.pos[camerafollowsprite].x;
            listenery = spriteparts.pos[camerafollowsprite].y;
        }

        dist = sqrt(sqr(emitterx - listenerx) + sqr(emittery - listenery)) / sound_maxdist;

        // play distant sounds
        if ((dist > 0.5) & (CVar::snd_effects_battle))
        {
            switch (samplenum)
            {
            case sfx_m79_explosion:
                fplaysound(sfx_dist_m79, listenerx, listenery, emitterx, emittery, chan);
                break;
            case sfx_grenade_explosion:
            case sfx_clustergrenade:
            case sfx_cluster_explosion:
                fplaysound(sfx_dist_grenade, listenerx, listenery, emitterx, emittery, chan);
                break;
            case sfx_ak74_fire:
            case sfx_m249_fire:
            case sfx_ruger77_fire:
            case sfx_spas12_fire:
            case sfx_deserteagle_fire:
            case sfx_steyraug_fire:
            case sfx_barretm82_fire:
            case sfx_minigun_fire:
            case sfx_colt1911_fire:
                fplaysound(81 + Random(4), listenerx, listenery, emitterx, emittery, chan);
                break;
            case sfx_dist_m79:
            case sfx_dist_grenade:
            case sfx_dist_gun1:
            case sfx_dist_gun2:
            case sfx_dist_gun3:
            case sfx_dist_gun4:
                if (dist > 1)
                    dist = dist - 1;
                else
                    dist = 1 - 2 * dist;
                break;
            }
        }

        // decrease volume if grenade effect
        if ((grenadeeffecttimer > 0) && (samplenum != sfx_hum))
            dist = (dist + 10) * (grenadeeffecttimer / 7);

        if (dist > 1)
            return;

        if ((samplenum == sfx_rocketz) || (samplenum == sfx_chainsaw_r) ||
            (samplenum == sfx_flamer))
        {
            looping = true;
        } // loop
        else
        {
            looping = false; // one time
        }

        if (chan >= reserved_sources)
        {
            return;
        }
        if (chan == -1)
        {
            for (i = reserved_sources; i <= max_sources - 1; i++)
            {
                if (!Engine.isValidVoiceHandle(sources[i]))
                {
                    chan = i;
                    break;
                }
                if (!Engine.getPause(sources[i]))
                {
                    continue;
                }
                //                alGetSourcei(sources[i], AL_SOURCE_STATE, &state);
                //                if (state != AL_PLAYING)
                {
                    chan = i;
                    break;
                }
            }
        }

        if (chan != -1)
        {
            if (Engine.isValidVoiceHandle(sources[chan]) && !Engine.getPause(sources[chan]))
            {
                return;
            }

            sources[chan] = Engine.play3d(*Waves[samplenum],
                                          (float)((emitterx - listenerx)) / sound_meterlength,
                                          (float)((emittery - listenery)) / sound_meterlength,
                                          (float)(-sound_panwidth) / sound_meterlength);
            auto volume = volumeinternal * (1.0f - dist);
            Engine.setVolume(sources[chan], volume);
            Engine.setLooping(sources[chan], looping);

            LogDebug(AUDIO, "Play sound {}; sample num {}; channel {}", sample_files[samplenum],
                     samplenum, chan);
        }
    }

    bool stopsound(std::int32_t channel)
    {
        Engine.stop(sources[channel]);
        return true;
    }

    bool setsoundpaused(std::int32_t channel, bool paused)
    {
        Engine.setPause(sources[channel], paused);
        return true;
    }

    bool setvolume(std::int32_t channel, float volume)
    {
        if (channel == -1)
        {
            Engine.setGlobalVolume(volume);
        }
        else
        {
            Engine.setVolume(sources[channel], volume);
        }
        return true;
    }

  private:
    SoLoud::Soloud Engine;
    PascalArray<SoLoud::Wav *, 1, max_samples> Waves = {};
    std::array<SoLoud::handle, max_sources> sources{};
};

SoundEngine Engine;

bool initsound()
{
    LogDebug(AUDIO, "InitSound");
    return Engine.InitSound();
}

tsoundsample loadsample(const std::string_view &name, tsoundsample samp)
{
    return Engine.LoadSample(name, samp);
}

static std::string uppercase(const std::string_view &str)
{
    std::string temp;
    temp.reserve(str.size());
    std::transform(str.begin(), str.end(), temp.begin(), ::toupper);
    return temp;
}

std::int8_t soundnametoid(const std::string &name)
{
    std::uint8_t i;

    std::int8_t result;
    result = -1;
    if (high(scriptsamp) < 0)
    {
        return result;
    }
    for (i = 0; i < scriptsamp.size(); i++)
    {
        if (uppercase(scriptsamp[i].name) == uppercase(name))
        {
            result = i;
            break;
        }
    }
    return result;
}

/* Takes a volume percentage (0-100) and converts it for internal use (0-1).
  The result is exponentially scaled to improve volume control intuitiveness
  and sensitivity at lower decibels.

  VolumeSetting the volume percentage to scale
  return the volume scaled for internal use
*/
float scalevolumesetting(std::uint8_t volumesetting)
{
    return (power(1.0404, volumesetting) - 1) / (1.0404 - 1) / 1275;
}

void loadsounds(string moddir)
{

    std::string sfxpath;
    std::int32_t i;

    sfxpath = moddir + "sfx/";

    // Sound effects
    GetMainConsole().console("Loading sound effects", debug_message_color);

    for (i = 1; i < sample_files.size(); i++)
    {
        if (sample_files[i] != "")
        {
            samp[i] = loadsample((sfxpath + sample_files[i]), samp[i]);
            if (!samp[i].loaded)
                GetMainConsole().console(string("Unable to load file ") + sfxpath + sample_files[i],
                                         debug_message_color);
        }
    }
}

void closesound()
{
    Engine.CloseSound();
}

void fplaysound(std::int32_t samplenum, float listenerx, float listenery, float emitterx,
                float emittery, std::int32_t chan)
{
    Engine.fplaysound(samplenum, listenerx, listenery, emitterx, emittery, chan);
}

void playsound(std::int32_t sample)
{
    fplaysound(sample, camerax, cameray, camerax, cameray, defaultchannel);
}

void playsound(std::int32_t sample, std::int32_t channel)
{
    fplaysound(sample, camerax, cameray, camerax, cameray, channel);
}

void playsound(std::int32_t sample, tvector2 &emitter)
{
    fplaysound(sample, camerax, cameray, emitter.x, emitter.y, defaultchannel);
}

void playsound(std::int32_t sample, tvector2 &emitter, int32_t channel)
{
    fplaysound(sample, camerax, cameray, emitter.x, emitter.y, channel);
}

bool stopsound(std::int32_t channel)
{
    return Engine.stopsound(channel);
}

bool setsoundpaused(std::int32_t channel, bool paused)
{
    return Engine.setsoundpaused(channel, paused);
}

bool setvolume(std::int32_t channel, float volume)
{
    return Engine.setvolume(channel, volume);
}
