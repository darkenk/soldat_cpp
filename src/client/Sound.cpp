// automatically converted

#include "Sound.hpp"
#include "Client.hpp"
#include "common/Logging.hpp"
#include "common/Parts.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/Constants.hpp"
#include "common/Console.hpp"
#include "shared/Cvar.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <SDL2/SDL.h>
#include <array>
#include <soloud.h>
#include <soloud_wav.h>
#include <string>

using string = std::string;

std::array<tsoundsample, ToUint32(SfxEffect::COUNT)> samp;
std::vector<tscriptsound> scriptsamp;
float volumeinternal = 0.5f;
std::int32_t defaultchannel = -1;

static std::string_view AUDIO = "audio";

constexpr void Add(std::array<std::string_view, ToUint32(SfxEffect::COUNT)> &ref, SfxEffect sample,
                   std::string_view name)
{
  auto &s = ref[ToUint32(sample)];
  if (not s.empty())
  {
    throw std::runtime_error("Exists " + std::string(name));
  }
  s = name;
}

constexpr auto GenerateSampleFileNames() -> std::array<std::string_view, ToUint32(SfxEffect::COUNT)>
{
  std::array<std::string_view, ToUint32(SfxEffect::COUNT)> ref;
  Add(ref, SfxEffect::ak74_fire, "ak74-fire.wav");
  Add(ref, SfxEffect::rocketz, "rocketz.wav");
  Add(ref, SfxEffect::ak74_reload, "ak74-reload.wav");
  Add(ref, SfxEffect::m249_fire, "m249-fire.wav");
  Add(ref, SfxEffect::ruger77_fire, "ruger77-fire.wav");
  Add(ref, SfxEffect::ruger77_reload, "ruger77-reload.wav");
  Add(ref, SfxEffect::m249_reload, "m249-reload.wav");
  Add(ref, SfxEffect::mp5_fire, "mp5-fire.wav");
  Add(ref, SfxEffect::mp5_reload, "mp5-reload.wav");
  Add(ref, SfxEffect::spas12_fire, "spas12-fire.wav");
  Add(ref, SfxEffect::spas12_reload, "spas12-reload.wav");
  Add(ref, SfxEffect::standup, "standup.wav");
  Add(ref, SfxEffect::fall, "fall.wav");
  Add(ref, SfxEffect::spawn, "spawn.wav");
  Add(ref, SfxEffect::m79_fire, "m79-fire.wav");
  Add(ref, SfxEffect::m79_explosion, "m79-explosion.wav");
  Add(ref, SfxEffect::m79_reload, "m79-reload.wav");
  Add(ref, SfxEffect::grenade_throw, "grenade-throw.wav");
  Add(ref, SfxEffect::grenade_explosion, "grenade-explosion.wav");
  Add(ref, SfxEffect::grenade_bounce, "grenade-bounce.wav");
  Add(ref, SfxEffect::bryzg, "bryzg.wav");
  Add(ref, SfxEffect::infiltmus, "infiltmus.wav");
  Add(ref, SfxEffect::headchop, "headchop.wav");
  Add(ref, SfxEffect::explosion_erg, "explosion-erg.wav");
  Add(ref, SfxEffect::water_step, "water-step.wav");
  Add(ref, SfxEffect::bulletby, "bulletby.wav");
  Add(ref, SfxEffect::bodyfall, "bodyfall.wav");
  Add(ref, SfxEffect::deserteagle_fire, "deserteagle-fire.wav");
  Add(ref, SfxEffect::deserteagle_reload, "deserteagle-reload.wav");
  Add(ref, SfxEffect::steyraug_fire, "steyraug-fire.wav");
  Add(ref, SfxEffect::steyraug_reload, "steyraug-reload.wav");
  Add(ref, SfxEffect::barretm82_fire, "barretm82-fire.wav");
  Add(ref, SfxEffect::barretm82_reload, "barretm82-reload.wav");
  Add(ref, SfxEffect::minigun_fire, "minigun-fire.wav");
  Add(ref, SfxEffect::minigun_reload, "minigun-reload.wav");
  Add(ref, SfxEffect::minigun_start, "minigun-start.wav");
  Add(ref, SfxEffect::minigun_end, "minigun-end.wav");
  Add(ref, SfxEffect::pickupgun, "pickupgun.wav");
  Add(ref, SfxEffect::capture, "capture.wav");
  Add(ref, SfxEffect::colt1911_fire, "colt1911-fire.wav");
  Add(ref, SfxEffect::colt1911_reload, "colt1911-reload.wav");
  Add(ref, SfxEffect::changeweapon, "changeweapon.wav");
  Add(ref, SfxEffect::shell, "shell.wav");
  Add(ref, SfxEffect::shell2, "shell2.wav");
  Add(ref, SfxEffect::dead_hit, "dead-hit.wav");
  Add(ref, SfxEffect::throwgun, "throwgun.wav");
  Add(ref, SfxEffect::bow_fire, "bow-fire.wav");
  Add(ref, SfxEffect::takebow, "takebow.wav");
  Add(ref, SfxEffect::takemedikit, "takemedikit.wav");
  Add(ref, SfxEffect::wermusic, "wermusic.wav");
  Add(ref, SfxEffect::ts, "ts.wav");
  Add(ref, SfxEffect::ctf, "ctf.wav");
  Add(ref, SfxEffect::berserker, "berserker.wav");
  Add(ref, SfxEffect::godflame, "godflame.wav");
  Add(ref, SfxEffect::flamer, "flamer.wav");
  Add(ref, SfxEffect::predator, "predator.wav");
  Add(ref, SfxEffect::killberserk, "killberserk.wav");
  Add(ref, SfxEffect::vesthit, "vesthit.wav");
  Add(ref, SfxEffect::burn, "burn.wav");
  Add(ref, SfxEffect::vesttake, "vesttake.wav");
  Add(ref, SfxEffect::clustergrenade, "clustergrenade.wav");
  Add(ref, SfxEffect::cluster_explosion, "cluster-explosion.wav");
  Add(ref, SfxEffect::grenade_pullout, "grenade-pullout.wav");
  Add(ref, SfxEffect::spit, "spit.wav");
  Add(ref, SfxEffect::stuff, "stuff.wav");
  Add(ref, SfxEffect::smoke, "smoke.wav");
  Add(ref, SfxEffect::match, "match.wav");
  Add(ref, SfxEffect::roar, "roar.wav");
  Add(ref, SfxEffect::step, "step.wav");
  Add(ref, SfxEffect::step2, "step2.wav");
  Add(ref, SfxEffect::step3, "step3.wav");
  Add(ref, SfxEffect::step4, "step4.wav");
  Add(ref, SfxEffect::hum, "hum.wav");
  Add(ref, SfxEffect::ric, "ric.wav");
  Add(ref, SfxEffect::ric2, "ric2.wav");
  Add(ref, SfxEffect::ric3, "ric3.wav");
  Add(ref, SfxEffect::ric4, "ric4.wav");
  Add(ref, SfxEffect::dist_m79, "dist-m79.wav");
  Add(ref, SfxEffect::dist_grenade, "dist-grenade.wav");
  Add(ref, SfxEffect::dist_gun1, "dist-gun1.wav");
  Add(ref, SfxEffect::dist_gun2, "dist-gun2.wav");
  Add(ref, SfxEffect::dist_gun3, "dist-gun3.wav");
  Add(ref, SfxEffect::dist_gun4, "dist-gun4.wav");
  Add(ref, SfxEffect::death, "death.wav");
  Add(ref, SfxEffect::death2, "death2.wav");
  Add(ref, SfxEffect::death3, "death3.wav");
  Add(ref, SfxEffect::crouch_move, "crouch-move.wav");
  Add(ref, SfxEffect::hit_arg, "hit-arg.wav");
  Add(ref, SfxEffect::hit_arg2, "hit-arg2.wav");
  Add(ref, SfxEffect::hit_arg3, "hit-arg3.wav");
  Add(ref, SfxEffect::goprone, "goprone.wav");
  Add(ref, SfxEffect::roll, "roll.wav");
  Add(ref, SfxEffect::fall_hard, "fall-hard.wav");
  Add(ref, SfxEffect::onfire, "onfire.wav");
  Add(ref, SfxEffect::firecrack, "firecrack.wav");
  Add(ref, SfxEffect::scope, "scope.wav");
  Add(ref, SfxEffect::scopeback, "scopeback.wav");
  Add(ref, SfxEffect::playerdeath, "playerdeath.wav");
  Add(ref, SfxEffect::changespin, "changespin.wav");
  Add(ref, SfxEffect::arg, "arg.wav");
  Add(ref, SfxEffect::lava, "lava.wav");
  Add(ref, SfxEffect::regenerate, "regenerate.wav");
  Add(ref, SfxEffect::prone_move, "prone-move.wav");
  Add(ref, SfxEffect::jump, "jump.wav");
  Add(ref, SfxEffect::crouch, "crouch.wav");
  Add(ref, SfxEffect::crouch_movel, "crouch-movel.wav");
  Add(ref, SfxEffect::step5, "step5.wav");
  Add(ref, SfxEffect::step6, "step6.wav");
  Add(ref, SfxEffect::step7, "step7.wav");
  Add(ref, SfxEffect::step8, "step8.wav");
  Add(ref, SfxEffect::stop, "stop.wav");
  Add(ref, SfxEffect::bulletby2, "bulletby2.wav");
  Add(ref, SfxEffect::bulletby3, "bulletby3.wav");
  Add(ref, SfxEffect::bulletby4, "bulletby4.wav");
  Add(ref, SfxEffect::bulletby5, "bulletby5.wav");
  Add(ref, SfxEffect::weaponhit, "weaponhit.wav");
  Add(ref, SfxEffect::clipfall, "clipfall.wav");
  Add(ref, SfxEffect::bonecrack, "bonecrack.wav");
  Add(ref, SfxEffect::gaugeshell, "gaugeshell.wav");
  Add(ref, SfxEffect::colliderhit, "colliderhit.wav");
  Add(ref, SfxEffect::kit_fall, "kit-fall.wav");
  Add(ref, SfxEffect::kit_fall2, "kit-fall2.wav");
  Add(ref, SfxEffect::flag, "flag.wav");
  Add(ref, SfxEffect::flag2, "flag2.wav");
  Add(ref, SfxEffect::takegun, "takegun.wav");
  Add(ref, SfxEffect::infilt_point, "infilt-point.wav");
  Add(ref, SfxEffect::menuclick, "menuclick.wav");
  Add(ref, SfxEffect::knife, "knife.wav");
  Add(ref, SfxEffect::slash, "slash.wav");
  Add(ref, SfxEffect::chainsaw_d, "chainsaw-d.wav");
  Add(ref, SfxEffect::chainsaw_m, "chainsaw-m.wav");
  Add(ref, SfxEffect::chainsaw_r, "chainsaw-r.wav");
  Add(ref, SfxEffect::piss, "piss.wav");
  Add(ref, SfxEffect::law, "law.wav");
  Add(ref, SfxEffect::chainsaw_o, "chainsaw-o.wav");
  Add(ref, SfxEffect::m2fire, "m2fire.wav");
  Add(ref, SfxEffect::m2explode, "m2explode.wav");
  Add(ref, SfxEffect::m2overheat, "m2overheat.wav");
  Add(ref, SfxEffect::signal, "signal.wav");
  Add(ref, SfxEffect::m2use, "m2use.wav");
  Add(ref, SfxEffect::scoperun, "scoperun.wav");
  Add(ref, SfxEffect::mercy, "mercy.wav");
  Add(ref, SfxEffect::ric5, "ric5.wav");
  Add(ref, SfxEffect::ric6, "ric6.wav");
  Add(ref, SfxEffect::ric7, "ric7.wav");
  Add(ref, SfxEffect::law_start, "law-start.wav");
  Add(ref, SfxEffect::law_end, "law-end.wav");
  Add(ref, SfxEffect::boomheadshot, "boomheadshot.wav");
  Add(ref, SfxEffect::snapshot, "snapshot.wav");
  Add(ref, SfxEffect::radio_efcup, "radio/efcup.wav");
  Add(ref, SfxEffect::radio_efcmid, "radio/efcmid.wav");
  Add(ref, SfxEffect::radio_efcdown, "radio/efcdown.wav");
  Add(ref, SfxEffect::radio_ffcup, "radio/ffcup.wav");
  Add(ref, SfxEffect::radio_ffcmid, "radio/ffcmid.wav");
  Add(ref, SfxEffect::radio_ffcdown, "radio/ffcdown.wav");
  Add(ref, SfxEffect::radio_esup, "radio/esup.wav");
  Add(ref, SfxEffect::radio_esmid, "radio/esmid.wav");
  Add(ref, SfxEffect::radio_esdown, "radio/esdown.wav");
  Add(ref, SfxEffect::bounce, "bounce.wav");
  Add(ref, SfxEffect::rain, "sfx_rain.wav");
  Add(ref, SfxEffect::snow, "sfx_snow.wav");
  Add(ref, SfxEffect::wind, "sfx_wind.wav");
  return ref;
}

constexpr auto sample_files = GenerateSampleFileNames();

class SoundEngine
{
public:
  auto InitSound() -> bool
  {
    LogDebug(AUDIO, "InitSound");
    if (Engine.init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::SDL2) != 0)
    {
      return false;
    }
    for (auto i = 0U; i < ToUint32(SfxEffect::COUNT); i++)
    {
      samp[i].loaded = false;
      samp[i].buffer = i;
      Waves[i] = nullptr;
    }
    Engine.setGlobalVolume(volumeinternal);
    return true;
  }

  auto LoadSample(const std::string_view &name, const tsoundsample &samp) -> tsoundsample
  {

    tsoundsample result;
    if (!samp.loaded)
    {
      result.loaded = false;
    }
    else
    {
      result = samp;
    }

    auto filebuffer = GS::GetFileSystem().ReadFile(name);
    if (filebuffer.empty())
    {
      LogWarn(AUDIO, "Empty audio {}", name);
      return result;
    }

    auto *wav = new SoLoud::Wav();
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
    std::for_each(std::begin(Waves), std::end(Waves), [](auto &w) {
      delete w;
      w = nullptr;
    });
    Engine.deinit();
  }

  void fplaysound(SfxEffect samplenum, float listenerx, float listenery, float emitterx,
                  float emittery, std::int32_t chan)
  {
    float dist;
    std::int32_t looping;
    std::int32_t i;
    // Pan: Single = 0.0;

    if (!samp[ToUint32(samplenum)].loaded)
    {
      return;
    }

    if (camerafollowsprite > 0)
    {
      const auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(camerafollowsprite);
      listenerx = spritePartsPos.x;
      listenery = spritePartsPos.y;
    }

    dist = sqrt(sqr(emitterx - listenerx) + sqr(emittery - listenery)) / sound_maxdist;

    // play distant sounds
    if ((dist > 0.5) && (CVar::snd_effects_battle))
    {
      switch (samplenum)
      {
      case SfxEffect::m79_explosion:
        fplaysound(SfxEffect::dist_m79, listenerx, listenery, emitterx, emittery, chan);
        break;
      case SfxEffect::grenade_explosion:
      case SfxEffect::clustergrenade:
      case SfxEffect::cluster_explosion:
        fplaysound(SfxEffect::dist_grenade, listenerx, listenery, emitterx, emittery, chan);
        break;
      case SfxEffect::ak74_fire:
      case SfxEffect::m249_fire:
      case SfxEffect::ruger77_fire:
      case SfxEffect::spas12_fire:
      case SfxEffect::deserteagle_fire:
      case SfxEffect::steyraug_fire:
      case SfxEffect::barretm82_fire:
      case SfxEffect::minigun_fire:
      case SfxEffect::colt1911_fire: {
        auto sound = SfxEffect::dist_gun1;
        switch (Random(4))
        {
        case 1:
          sound = SfxEffect::dist_gun1;
          break;
        case 2:
          sound = SfxEffect::dist_gun2;
          break;
        case 3:
          sound = SfxEffect::dist_gun3;
          break;
        }

        fplaysound(sound, listenerx, listenery, emitterx, emittery, chan);
      }
      break;
      case SfxEffect::dist_m79:
      case SfxEffect::dist_grenade:
      case SfxEffect::dist_gun1:
      case SfxEffect::dist_gun2:
      case SfxEffect::dist_gun3:
      case SfxEffect::dist_gun4:
        if (dist > 1)
        {
          dist = dist - 1;
        }
        else
        {
          dist = 1 - 2 * dist;
        }
        break;
      default:
        break;
      }
    }

    // decrease volume if grenade effect
    if ((grenadeeffecttimer > 0) && (samplenum != SfxEffect::hum))
    {
      dist = (dist + 10) * (grenadeeffecttimer / 7);
    }

    if (dist > 1)
    {
      return;
    }

    if ((samplenum == SfxEffect::rocketz) || (samplenum == SfxEffect::chainsaw_r) ||
        (samplenum == SfxEffect::flamer))
    {
      looping = 1;
    } // loop
    else
    {
      looping = 0; // one time
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

      sources[chan] = Engine.play3d(
        *Waves[ToUint32(samplenum)], (emitterx - listenerx) / sound_meterlength,
        (emittery - listenery) / sound_meterlength, (float)(-sound_panwidth) / sound_meterlength);
      auto volume = volumeinternal * (1.0f - dist);
      Engine.setVolume(sources[chan], volume);
      Engine.setLooping(sources[chan], looping != 0);

      LogDebug(AUDIO, "Play sound {}; sample num {}; channel {}", sample_files[ToUint32(samplenum)],
               samplenum, chan);
    }
  }

  auto stopsound(std::int32_t channel) -> bool
  {
    Engine.stop(sources[channel]);
    return true;
  }

  auto setsoundpaused(std::int32_t channel, bool paused) -> bool
  {
    Engine.setPause(sources[channel], paused);
    return true;
  }

  auto setvolume(std::int32_t channel, float volume) -> bool
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
  std::array<SoLoud::Wav *, ToUint32(SfxEffect::COUNT)> Waves = {};
  std::array<SoLoud::handle, max_sources> sources{};
};

SoundEngine Engine;

auto initsound() -> bool
{
  LogDebug(AUDIO, "InitSound");
  return Engine.InitSound();
}

auto loadsample(const std::string_view &name, const tsoundsample &samp) -> tsoundsample
{
  return Engine.LoadSample(name, samp);
}

static auto uppercase(const std::string_view &str) -> std::string
{
  std::string temp;
  temp.reserve(str.size());
  std::transform(str.begin(), str.end(), temp.begin(), ::toupper);
  return temp;
}

auto soundnametoid(const std::string &name) -> std::int8_t
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
auto scalevolumesetting(const std::uint8_t volumesetting) -> float
{
  return (power(1.0404, volumesetting) - 1) / (1.0404 - 1) / 1275;
}

void loadsounds(const string &moddir)
{

  std::string sfxpath;
  std::int32_t i;

  sfxpath = moddir + "sfx/";

  // Sound effects
  GS::GetMainConsole().console("Loading sound effects", debug_message_color);

  for (i = 1U; i < sample_files.size(); i++)
  {
    if (!sample_files[i].empty())
    {
      samp[i] = loadsample((sfxpath + sample_files[i].data()), samp[i]);
      if (!samp[i].loaded)
      {
        GS::GetMainConsole().console(
          string("Unable to load file ") + sfxpath + sample_files[i].data(), debug_message_color);
      }
    }
  }
}

void closesound()
{
  Engine.CloseSound();
}

void fplaysound(SfxEffect samplenum, float listenerx, float listenery, float emitterx,
                float emittery, std::int32_t chan)
{
  Engine.fplaysound(samplenum, listenerx, listenery, emitterx, emittery, chan);
}

void playsound(SfxEffect sample)
{
  fplaysound(sample, camerax, cameray, camerax, cameray, defaultchannel);
}

void playsound(SfxEffect sample, std::int32_t channel)
{
  fplaysound(sample, camerax, cameray, camerax, cameray, channel);
}

void playsound(SfxEffect sample, const tvector2 &emitter)
{
  fplaysound(sample, camerax, cameray, emitter.x, emitter.y, defaultchannel);
}

void playsound(SfxEffect sample, const tvector2 &emitter, int32_t channel)
{
  fplaysound(sample, camerax, cameray, emitter.x, emitter.y, channel);
}

auto stopsound(std::int32_t channel) -> bool { return Engine.stopsound(channel); }

auto setsoundpaused(std::int32_t channel, bool paused) -> bool
{
  return Engine.setsoundpaused(channel, paused);
}

auto setvolume(std::int32_t channel, float volume) -> bool
{
  return Engine.setvolume(channel, volume);
}
