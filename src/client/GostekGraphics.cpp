// automatically converted

#include "GostekGraphics.hpp"
#include "GameRendering.hpp"
#include "Gfx.hpp"
#include "common/gfx.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/Cvar.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <Tracy.hpp>
#include <array>
#include <memory_resource>
#include <numbers>
#include <set>

using std::numbers::pi;

constexpr std::int32_t color_none = 0;
constexpr std::int32_t color_main = 1;
constexpr std::int32_t color_pants = 2;
constexpr std::int32_t color_skin = 3;
constexpr std::int32_t color_hair = 4;
constexpr std::int32_t color_cygar = 5;
constexpr std::int32_t color_headblood = 6;

constexpr std::int32_t alpha_base = 0;
constexpr std::int32_t alpha_blood = 1;
constexpr std::int32_t alpha_nades = 2;

struct GostekSprite
{
public:
  constexpr GostekSprite(const std::string_view id, const std::int32_t image, const std::uint8_t p1,
                         const std::uint8_t p2, const float cx, const float cy,
                         const std::uint8_t visible, const std::uint8_t flip,
                         const std::uint8_t team, const float flex, const std::uint8_t color,
                         const std::uint8_t alpha)
    : id(id), image(image), p1(p1), p2(p2), cx(cx), cy(cy), flex(flex), flip(flip != 0u),
      team(team != 0u), color(color), alpha(alpha), visible(visible == 1)
  {
  }

  const std::string_view id;
  const std::int32_t image;
  const std::uint8_t p1;
  const std::uint8_t p2;
  float cx;
  float cy;
  const float flex;
  const bool flip;
  const bool team;
  const std::uint8_t color;
  const std::uint8_t alpha;
  const bool visible;
};

static constexpr std::array<GostekSprite, 131> sGostekSpritesDefaults = {
  GostekSprite("Secondary_Deagles", 0, 5, 10, 0.3, 0.5, 0, 1, 0, 0, color_none, alpha_base),
  GostekSprite("Secondary_Mp5", GFX::WEAPONS_MP5, 5, 10, 0.3, 0.3, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Secondary_Ak74", GFX::WEAPONS_AK74, 5, 10, 0.3, 0.25, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Secondary_Steyr", GFX::WEAPONS_STEYR, 5, 10, 0.3, 0.5, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Secondary_Spas", GFX::WEAPONS_SPAS, 5, 10, 0.3, 0.3, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Secondary_Ruger", GFX::WEAPONS_RUGER, 5, 10, 0.3, 0.3, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Secondary_M79", GFX::WEAPONS_M79, 5, 10, 0.3, 0.35, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Secondary_Barrett", GFX::WEAPONS_BARRETT, 5, 10, 0.3, 0.35, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Secondary_Minimi", GFX::WEAPONS_MINIMI, 5, 10, 0.3, 0.35, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Secondary_Minigun", GFX::WEAPONS_MINIGUN, 5, 10, 0.2, 0.5, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Secondary_Socom", 0, 5, 10, 0.3, 0.5, 0, 1, 0, 0, color_none, alpha_base),

  GostekSprite("Secondary_Knife", 0, 5, 10, 0.3, 0.5, 0, 1, 0, 0, color_none, alpha_base),

  GostekSprite("Secondary_Chainsaw", GFX::WEAPONS_CHAINSAW, 5, 10, 0.25, 0.5, 0, 1, 0, 0,
               color_none, alpha_base),

  GostekSprite("Secondary_Law", GFX::WEAPONS_LAW, 5, 10, 0.3, 0.45, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Secondary_Flamebow", 0, 5, 10, 0.3, 0.5, 0, 1, 0, 0, color_none, alpha_base),

  GostekSprite("Secondary_Bow", 0, 5, 10, 0.3, 0.5, 0, 1, 0, 0, color_none, alpha_base),

  GostekSprite("Secondary_Flamer", GFX::WEAPONS_FLAMER, 5, 10, 0.3, 0.3, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Left_Thigh", GFX::GOSTEK_UDO, 6, 3, 0.2, 0.5, 1, 1, 1, 5, color_pants, alpha_base),

  GostekSprite("Left_Thigh_Dmg", GFX::GOSTEK_RANNY_UDO, 6, 3, 0.2, 0.5, 0, 1, 1, 5, color_none,
               alpha_blood),

  GostekSprite("Left_Foot", GFX::GOSTEK_STOPA, 2, 18, 0.35, 0.35, 1, 1, 1, 0, color_none,
               alpha_base),

  GostekSprite("Left_Jetfoot", GFX::GOSTEK_LECISTOPA, 2, 18, 0.35, 0.35, 0, 1, 1, 0, color_none,
               alpha_base),

  GostekSprite("Left_Lowerleg", GFX::GOSTEK_NOGA, 3, 2, 0.15, 0.55, 1, 1, 1, 0, color_pants,
               alpha_base),

  GostekSprite("Left_Lowerleg_Dmg", GFX::GOSTEK_RANNY_NOGA, 3, 2, 0.15, 0.55, 0, 1, 1, 0,
               color_none, alpha_blood),

  GostekSprite("Left_Arm", GFX::GOSTEK_RAMIE, 11, 14, 0, 0.5, 1, 1, 1, 0, color_main, alpha_base),

  GostekSprite("Left_Arm_Dmg", GFX::GOSTEK_RANNY_RAMIE, 11, 14, 0, 0.5, 0, 1, 1, 0, color_none,
               alpha_blood),

  GostekSprite("Left_Forearm", GFX::GOSTEK_REKA, 14, 15, 0, 0.5, 1, 0, 1, 5, color_main,
               alpha_base),

  GostekSprite("Left_Forearm_Dmg", GFX::GOSTEK_RANNY_REKA, 14, 15, 0, 0.5, 0, 1, 1, 5, color_none,
               alpha_blood),

  GostekSprite("Left_Hand", GFX::GOSTEK_DLON, 15, 19, 0, 0.4, 1, 1, 1, 0, color_skin, alpha_base),

  GostekSprite("Grabbed_Helmet", GFX::GOSTEK_HELM, 15, 19, 0, 0.5, 0, 1, 1, 0, color_main,
               alpha_base),

  GostekSprite("Grabbed_Hat", GFX::GOSTEK_KAP, 15, 19, 0.1, 0.4, 0, 1, 1, 0, color_main,
               alpha_base),

  GostekSprite("Right_Thigh", GFX::GOSTEK_UDO, 5, 4, 0.2, 0.65, 1, 1, 1, 5, color_pants,
               alpha_base),

  GostekSprite("Right_Thigh_Dmg", GFX::GOSTEK_RANNY_UDO, 5, 4, 0.2, 0.65, 0, 1, 1, 5, color_none,
               alpha_blood),

  GostekSprite("Right_Foot", GFX::GOSTEK_STOPA, 1, 17, 0.35, 0.35, 1, 1, 1, 0, color_none,
               alpha_base),

  GostekSprite("Right_Jetfoot", GFX::GOSTEK_LECISTOPA, 1, 17, 0.35, 0.35, 0, 1, 1, 0, color_none,
               alpha_base),

  GostekSprite("Right_Lowerleg", GFX::GOSTEK_NOGA, 4, 1, 0.15, 0.55, 1, 1, 1, 0, color_pants,
               alpha_base),

  GostekSprite("Right_Lowerleg_Dmg", GFX::GOSTEK_RANNY_NOGA, 4, 1, 0.15, 0.55, 0, 1, 1, 0,
               color_none, alpha_blood),

  GostekSprite("Chest", GFX::GOSTEK_KLATA, 10, 11, 0.1, 0.3, 1, 1, 1, 0, color_main, alpha_base),

  GostekSprite("Vest", GFX::GOSTEK_KAMIZELKA, 10, 11, 0.1, 0.3, 0, 1, 1, 0, color_none, alpha_base),

  GostekSprite("Chest_Dmg", GFX::GOSTEK_RANNY_KLATA, 10, 11, 0.1, 0.3, 0, 1, 1, 0, color_none,
               alpha_blood),

  GostekSprite("Hip", GFX::GOSTEK_BIODRO, 5, 6, 0.25, 0.6, 1, 1, 1, 0, color_main, alpha_base),

  GostekSprite("Hip_Dmg", GFX::GOSTEK_RANNY_BIODRO, 5, 6, 0.25, 0.6, 0, 1, 1, 0, color_none,
               alpha_blood),

  GostekSprite("Head", GFX::GOSTEK_MORDA, 9, 12, 0, 0.5, 1, 1, 1, 0, color_skin, alpha_base),

  GostekSprite("Head_Dmg", GFX::GOSTEK_RANNY_MORDA, 9, 12, 0, 0.5, 0, 1, 1, 0, color_headblood,
               alpha_blood),

  GostekSprite("Head_Dead", GFX::GOSTEK_MORDA, 9, 12, 0.5, 0.5, 0, 1, 1, 0, color_skin, alpha_base),

  GostekSprite("Head_Dead_Dmg", GFX::GOSTEK_RANNY_MORDA, 9, 12, 0.5, 0.5, 0, 1, 1, 0,
               color_headblood, alpha_blood),

  GostekSprite("Mr_T", GFX::GOSTEK_HAIR3, 9, 12, 0, 0.5, 0, 1, 1, 0, color_hair, alpha_base),

  GostekSprite("Helmet", GFX::GOSTEK_HELM, 9, 12, 0, 0.5, 0, 1, 1, 0, color_main, alpha_base),

  GostekSprite("Hat", GFX::GOSTEK_KAP, 9, 12, 0, 0.5, 0, 1, 1, 0, color_main, alpha_base),

  GostekSprite("Rambo_Badge", GFX::GOSTEK_BADGE, 9, 12, 0, 0.5, 0, 1, 1, 0, color_none, alpha_base),

  GostekSprite("Hair_Dreadlocks", GFX::GOSTEK_HAIR1, 9, 12, 0, 0.5, 0, 1, 1, 0, color_hair,
               alpha_base),

  GostekSprite("Hair_Dreadlock1", GFX::GOSTEK_DRED, 23, 24, 0, 1.22, 0, 0, 1, 0, color_hair,
               alpha_base),

  GostekSprite("Hair_Dreadlock2", GFX::GOSTEK_DRED, 23, 24, 0.1, 0.5, 0, 0, 1, 0, color_hair,
               alpha_base),

  GostekSprite("Hair_Dreadlock3", GFX::GOSTEK_DRED, 23, 24, 0.04, -0.3, 0, 0, 1, 0, color_hair,
               alpha_base),

  GostekSprite("Hair_Dreadlock4", GFX::GOSTEK_DRED, 23, 24, 0, -0.9, 0, 0, 1, 0, color_hair,
               alpha_base),

  GostekSprite("Hair_Dreadlock5", GFX::GOSTEK_DRED, 23, 24, -0.2, -1.35, 0, 0, 1, 0, color_hair,
               alpha_base),

  GostekSprite("Hair_Punk", GFX::GOSTEK_HAIR2, 9, 12, 0, 0.5, 0, 1, 1, 0, color_hair, alpha_base),

  GostekSprite("Hair_Normal", GFX::GOSTEK_HAIR4, 9, 12, 0, 0.5, 0, 1, 1, 0, color_hair, alpha_base),

  GostekSprite("Cigar", GFX::GOSTEK_CYGARO, 9, 12, -0.125, 0.4, 0, 1, 1, 0, color_cygar,
               alpha_base),

  GostekSprite("Silver_Lchain", GFX::GOSTEK_LANCUCH, 10, 22, 0.1, 0.5, 0, 0, 1, 0, color_none,
               alpha_base),

  GostekSprite("Silver_Rchain", GFX::GOSTEK_LANCUCH, 11, 22, 0.1, 0.5, 0, 0, 1, 0, color_none,
               alpha_base),

  GostekSprite("Silver_Pendant", GFX::GOSTEK_METAL, 22, 21, 0.5, 0.7, 0, 1, 1, 0, color_none,
               alpha_base),

  GostekSprite("Golden_Lchain", GFX::GOSTEK_ZLOTYLANCUCH, 10, 22, 0.1, 0.5, 0, 0, 1, 0, color_none,
               alpha_base),

  GostekSprite("Golden_Rchain", GFX::GOSTEK_ZLOTYLANCUCH, 11, 22, 0.1, 0.5, 0, 0, 1, 0, color_none,
               alpha_base),

  GostekSprite("Golden_Pendant", GFX::GOSTEK_ZLOTO, 22, 21, 0.5, 0.5, 0, 1, 1, 0, color_none,
               alpha_base),

  GostekSprite("Frag_Grenade1", GFX::WEAPONS_FRAG_GRENADE, 5, 6, 0.5, 0.1, 0, 0, 0, 0, color_none,
               alpha_nades),

  GostekSprite("Frag_Grenade2", GFX::WEAPONS_FRAG_GRENADE, 5, 6, 0.5, 0.1, 0, 0, 0, 0, color_none,
               alpha_nades),

  GostekSprite("Frag_Grenade3", GFX::WEAPONS_FRAG_GRENADE, 5, 6, 0.5, 0.1, 0, 0, 0, 0, color_none,
               alpha_nades),

  GostekSprite("Frag_Grenade4", GFX::WEAPONS_FRAG_GRENADE, 5, 6, 0.5, 0.1, 0, 0, 0, 0, color_none,
               alpha_nades),

  GostekSprite("Frag_Grenade5", GFX::WEAPONS_FRAG_GRENADE, 5, 6, 0.5, 0.1, 0, 0, 0, 0, color_none,
               alpha_nades),

  GostekSprite("Cluster_Grenade1", GFX::WEAPONS_CLUSTER_GRENADE, 5, 6, 0.5, 0.3, 0, 0, 0, 0,
               color_none, alpha_nades),

  GostekSprite("Cluster_Grenade2", GFX::WEAPONS_CLUSTER_GRENADE, 5, 6, 0.5, 0.3, 0, 0, 0, 0,
               color_none, alpha_nades),

  GostekSprite("Cluster_Grenade3", GFX::WEAPONS_CLUSTER_GRENADE, 5, 6, 0.5, 0.3, 0, 0, 0, 0,
               color_none, alpha_nades),

  GostekSprite("Cluster_Grenade4", GFX::WEAPONS_CLUSTER_GRENADE, 5, 6, 0.5, 0.3, 0, 0, 0, 0,
               color_none, alpha_nades),

  GostekSprite("Cluster_Grenade5", GFX::WEAPONS_CLUSTER_GRENADE, 5, 6, 0.5, 0.3, 0, 0, 0, 0,
               color_none, alpha_nades),

  GostekSprite("Primary_Deagles", GFX::WEAPONS_DEAGLES, 16, 15, 0.1, 0.8, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Deagles_Clip", GFX::WEAPONS_DEAGLES_CLIP, 16, 15, 0.1, 0.8, 0, 1, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Deagles_Fire", GFX::WEAPONS_DEAGLES_FIRE, 16, 15, -0.5, 1, 0, 0, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Mp5", GFX::WEAPONS_MP5, 16, 15, 0.15, 0.6, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Mp5_Clip", GFX::WEAPONS_MP5_CLIP, 16, 15, 0.15, 0.6, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Mp5_Fire", GFX::WEAPONS_MP5_FIRE, 16, 15, -0.65, 0.85, 0, 0, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Ak74", GFX::WEAPONS_AK74, 16, 15, 0.15, 0.5, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Ak74_Clip", GFX::WEAPONS_AK74_CLIP, 16, 15, 0.15, 0.5, 0, 1, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Ak74_Fire", GFX::WEAPONS_AK74_FIRE, 16, 15, -0.37, 0.8, 0, 0, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Steyr", GFX::WEAPONS_STEYR, 16, 15, 0.2, 0.6, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Steyr_Clip", GFX::WEAPONS_STEYR_CLIP, 16, 15, 0.2, 0.6, 0, 1, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Steyr_Fire", GFX::WEAPONS_STEYR_FIRE, 16, 15, -0.24, 0.75, 0, 0, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Spas", GFX::WEAPONS_SPAS, 16, 15, 0.1, 0.6, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Spas_Clip", 0, 16, 15, 0, 0, 0, 1, 0, 0, color_none, alpha_base),

  GostekSprite("Primary_Spas_Fire", GFX::WEAPONS_SPAS_FIRE, 16, 15, -0.2, 0.9, 0, 0, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Ruger", GFX::WEAPONS_RUGER, 16, 15, 0.1, 0.7, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Ruger_Clip", 0, 16, 15, 0, 0, 0, 1, 0, 0, color_none, alpha_base),

  GostekSprite("Primary_Ruger_Fire", GFX::WEAPONS_RUGER_FIRE, 16, 15, -0.35, 0.85, 0, 0, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_M79", GFX::WEAPONS_M79, 16, 15, 0.1, 0.7, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_M79_Clip", GFX::WEAPONS_M79_CLIP, 16, 15, 0.1, 0.7, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_M79_Fire", GFX::WEAPONS_M79_FIRE, 16, 15, -0.4, 0.8, 0, 0, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Barrett", GFX::WEAPONS_BARRETT, 16, 15, 0.15, 0.7, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Barrett_Clip", GFX::WEAPONS_BARRETT_CLIP, 16, 15, 0.15, 0.7, 0, 1, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Barrett_Fire", GFX::WEAPONS_BARRETT_FIRE, 16, 15, -0.15, 0.8, 0, 0, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Minimi", GFX::WEAPONS_MINIMI, 16, 15, 0.15, 0.6, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Minimi_Clip", GFX::WEAPONS_MINIMI_CLIP, 16, 15, 0.15, 0.6, 0, 1, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Minimi_Fire", GFX::WEAPONS_MINIMI_FIRE, 16, 15, -0.2, 0.9, 0, 0, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Minigun_Clip", GFX::WEAPONS_MINIGUN_CLIP, 8, 7, 0.5, 0.1, 0, 0, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Minigun", GFX::WEAPONS_MINIGUN, 16, 15, 0.05, 0.5, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Minigun_Fire", GFX::WEAPONS_MINIGUN_FIRE, 16, 15, -0.2, 0.45, 0, 0, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Socom", GFX::WEAPONS_SOCOM, 16, 15, 0.2, 0.55, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Socom_Clip", GFX::WEAPONS_SOCOM_CLIP, 16, 15, 0.2, 0.55, 0, 1, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Socom_Fire", GFX::WEAPONS_SOCOM_FIRE, 16, 15, -0.24, 0.85, 0, 0, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Knife", GFX::WEAPONS_KNIFE, 16, 20, -0.1, 0.6, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Knife_Clip", 0, 16, 15, 0, 0, 0, 1, 0, 0, color_none, alpha_base),

  GostekSprite("Primary_Knife_Fire", 0, 16, 15, 0, 0, 0, 1, 0, 0, color_none, alpha_base),

  GostekSprite("Primary_Chainsaw", GFX::WEAPONS_CHAINSAW, 16, 15, 0.1, 0.5, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Chainsaw_Clip", 0, 16, 15, 0, 0, 0, 1, 0, 0, color_none, alpha_base),

  GostekSprite("Primary_Chainsaw_Fire", GFX::WEAPONS_CHAINSAW_FIRE, 16, 15, 0, 0, 0, 0, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Law", GFX::WEAPONS_LAW, 16, 15, 0.1, 0.6, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Law_Clip", 0, 16, 15, 0, 0, 0, 1, 0, 0, color_none, alpha_base),

  GostekSprite("Primary_Law_Fire", GFX::WEAPONS_LAW_FIRE, 16, 15, -0.1, 0.55, 0, 0, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Bow", GFX::WEAPONS_BOW, 16, 15, -0.4, 0.55, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Bow_Arrow", GFX::WEAPONS_BOW_A, 16, 15, 0, 0.55, 0, 0, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Bow_String", GFX::WEAPONS_BOW_S, 16, 15, -0.4, 0.55, 0, 0, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Bow_Reload", GFX::WEAPONS_BOW, 16, 15, -0.4, 0.55, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Bow_Arrow_Reload", GFX::WEAPONS_ARROW, 16, 20, 0, 0.55, 0, 0, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Bow_String_Reload", GFX::WEAPONS_BOW_S, 16, 15, -0.4, 0.55, 0, 0, 0, 0,
               color_none, alpha_base),

  GostekSprite("Primary_Bow_Fire", GFX::WEAPONS_BOW_FIRE, 16, 15, 0, 0, 0, 0, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Flamer", GFX::WEAPONS_FLAMER, 16, 15, 0.2, 0.7, 0, 1, 0, 0, color_none,
               alpha_base),

  GostekSprite("Primary_Flamer_Clip", 0, 16, 15, 0, 0, 0, 1, 0, 0, color_none, alpha_base),

  GostekSprite("Primary_Flamer_Fire", GFX::WEAPONS_FLAMER_FIRE, 16, 15, 0, 0, 0, 0, 0, 0,
               color_none, alpha_base),

  GostekSprite("Right_Arm", GFX::GOSTEK_RAMIE, 10, 13, 0, 0.6, 1, 1, 1, 0, color_main, alpha_base),

  GostekSprite("Right_Arm_Dmg", GFX::GOSTEK_RANNY_RAMIE, 10, 13, -0.1, 0.5, 0, 1, 1, 0, color_none,
               alpha_blood),

  GostekSprite("Right_Forearm", GFX::GOSTEK_REKA, 13, 16, 0, 0.6, 1, 0, 1, 5, color_main,
               alpha_base),

  GostekSprite("Right_Forearm_Dmg", GFX::GOSTEK_RANNY_REKA, 13, 16, 0, 0.6, 0, 1, 1, 5, color_none,
               alpha_blood),

  GostekSprite("Right_Hand", GFX::GOSTEK_DLON, 16, 20, 0, 0.5, 1, 1, 1, 0, color_skin, alpha_base),
};

using tgostekspriteset = std::set<std::int32_t>;

static std::vector<GostekSprite> gosteksprites;
static tgostekspriteset gostekbase;

static void include(tgostekspriteset &sprite, std::int32_t v) { sprite.emplace(v); }

static void loaddefaults()
{
  gosteksprites.clear();
  gosteksprites.reserve(sGostekSpritesDefaults.size());
  gostekbase.clear();
  for (auto i = 0; i < sGostekSpritesDefaults.size(); i++)
  {
    gosteksprites.emplace_back(sGostekSpritesDefaults[i]);
    if (sGostekSpritesDefaults[i].visible)
    {
      gostekbase.emplace(i);
    }
  }
}

void loadgostekdata(const TIniFile::Entries &data)
{
  loaddefaults();

  for (std::int32_t i = GOSTEK_FIRST; i < GOSTEK_LAST; i++)
  {
    std::string cx = data.at(std::string(gosteksprites[i].id) + "_CenterX");
    std::string cy = data.at(std::string(gosteksprites[i].id) + "_CenterY");
    gosteksprites[i].cx = strtofloatdef(cx, gosteksprites[i].cx);
    gosteksprites[i].cy = strtofloatdef(cy, gosteksprites[i].cy);
  }
}

void applygostekconstraints(tgfxspritearray textures)
{
  auto texwidth = [&textures](const std::int32_t index) {
    return fabs(textures[index]->width * textures[index]->scale);
  };

  auto texheight = [&textures](const std::int32_t index) {
    return fabs(textures[index]->height * textures[index]->scale);
  };

  constexpr std::int32_t t2 = GFX::GOSTEK_TEAM2_STOPA - GFX::GOSTEK_STOPA;

  for (std::int32_t i = GOSTEK_FIRST; i < GOSTEK_LAST; i++)
  {
    auto &gs = gosteksprites[i];

    if (gs.image != 0)
    {
      auto w = texwidth(gs.image);
      w = max(w, texwidth(gs.image + ord(gs.flip)));
      w = max(w, texwidth(gs.image + t2 * ord(gs.team)));
      w = max(w, texwidth(gs.image + t2 * ord(gs.team) + ord(gs.flip)));

      auto h = texheight(gs.image);
      h = max(h, texheight(gs.image + ord(gs.flip)));
      h = max(h, texheight(gs.image + t2 * ord(gs.team)));
      h = max(h, texheight(gs.image + t2 * ord(gs.team) + ord(gs.flip)));

      const float cx = w * fabs(gs.cx + 0.5);
      const float cy = h * fabs(gs.cy + 0.5);

      if (cx > (w + gos_restrict_width))
      {
        gs.cx = 0.5 + sign(gs.cx + 0.5) * ((float)((w + gos_restrict_width)) / w);
      }

      if (cy > (h + gos_restrict_height))
      {
        gs.cy = 0.5 + sign(gs.cy + 0.5) * ((float)((h + gos_restrict_height)) / h);
      }
    }
  }
}

static void drawgosteksprite(pgfxsprite sprite, float x, float y, float sx, float sy, float cx, float cy,
                      float r, const tgfxcolor &color)
{
  ZoneScopedN("DrawGostekSprite");
  tgfxmat3 m;
  std::array<tgfxvertex, 4> buff;
  std::pmr::monotonic_buffer_resource res(buff.data(), buff.size() * sizeof(tgfxvertex));
  std::pmr::vector<tgfxvertex> v{4, &res};

  std::array<tgfxvertex, 4> buff2;
  std::pmr::monotonic_buffer_resource res2(buff2.data(), buff2.size() * sizeof(tvector2));
  std::pmr::vector<tvector2> p{4, &res2};

  const float c = cos(r);
  const float s = sin(r);

  m[0] = c * sx;
  m[3] = -s * sy;
  m[6] = x - cy * m[3] - cx * m[0];
  m[1] = s * sx;
  m[4] = c * sy;
  m[7] = y - cy * m[4] - cx * m[1];
  m[2] = 0;
  m[5] = 0;
  m[8] = 1;

  const float w = sprite->scale * sprite->width;
  const float h = sprite->scale * sprite->height;

  const float u0 = sprite->texcoords.left;
  const float v0 = sprite->texcoords.top;
  const float u1 = sprite->texcoords.right;
  const float v1 = sprite->texcoords.bottom;

  p[0] = gfxmat3mul(m, 0, 0);
  p[1] = gfxmat3mul(m, w, 0);
  p[2] = gfxmat3mul(m, w, h);
  p[3] = gfxmat3mul(m, 0, h);

  v[0] = gfxvertex(p[0].x, p[0].y, u0, v0, color);
  v[1] = gfxvertex(p[1].x, p[1].y, u1, v0, color);
  v[2] = gfxvertex(p[2].x, p[2].y, u1, v1, color);
  v[3] = gfxvertex(p[3].x, p[3].y, u0, v1, color);

  gfxdrawquad(sprite->texture, v);
}

void rendergostek(tsprite &soldier)
{
  ZoneScopedN("RenderGostek");
  std::int32_t i;
  std::int32_t index;
  bool showclip;
  tgostekspriteset visible;
  PascalArray<tgfxcolor, color_none, color_headblood> color;
  PascalArray<std::uint8_t, alpha_base, alpha_nades> alpha;
  tgfxmat3 m{0};
  float x1;
  float y1;
  float x2;
  float y2;
  float r;

  if ((soldier.style != tsprite::Style::Default) ||
      (soldier.ceasefirecounter > GS::GetGame().GetCeasefiretime() - 5) ||
      ((CVar::sv_realisticmode) && (soldier.visible == 0)) or (soldier.isspectator()) or
      (soldier.player->name.empty()) or (soldier.player->demoplayer))
  {
    return;
  }

  {
    ZoneScopedN("CopyGostekBase");
    visible = gostekbase;
  }

  // setup colors

  color[color_none] = rgba(0xffffff);
  color[color_main] = rgba(soldier.player->shirtcolor);
  color[color_pants] = rgba(soldier.player->pantscolor);
  color[color_skin] = rgba(soldier.player->skincolor);
  color[color_hair] = rgba(soldier.player->haircolor);
  color[color_cygar] = rgba(0xffffff);
  color[color_headblood] = rgba(0xaca9a8);

  if (soldier.hascigar == 5)
  {
    color[color_cygar] = rgba(0x616161);
  }

  alpha[alpha_base] = soldier.alpha;
  alpha[alpha_blood] = max(0.0, min(255.0, 200 - round(soldier.GetHealth())));

  if (soldier.GetHealth() > (90 - 40 * static_cast<int>(CVar::sv_realisticmode)))
  {
    alpha[alpha_blood] = 0;
  }

  if ((CVar::sv_realisticmode) && (soldier.visible > 0) && (soldier.visible < 45) &&
      (soldier.alpha > 60))
  {
    soldier.alpha = 3 * soldier.visible;
    alpha[alpha_base] = soldier.alpha;
    alpha[alpha_blood] = 0;
  }

  alpha[alpha_nades] = trunc(0.75 * alpha[alpha_base]);

  // blood
  if (alpha[alpha_blood] > 0)
  {
    visible.emplace(GOSTEK_LEFT_THIGH_DMG);
    visible.emplace(GOSTEK_LEFT_LOWERLEG_DMG);
    visible.emplace(GOSTEK_LEFT_FOREARM_DMG);
    visible.emplace(GOSTEK_LEFT_ARM_DMG);
    visible.emplace(GOSTEK_CHEST_DMG);
    visible.emplace(GOSTEK_HIP_DMG);
    visible.emplace(GOSTEK_HEAD_DMG);
    visible.emplace(GOSTEK_RIGHT_THIGH_DMG);
    visible.emplace(GOSTEK_RIGHT_LOWERLEG_DMG);
    visible.emplace(GOSTEK_RIGHT_FOREARM_DMG);
    visible.emplace(GOSTEK_RIGHT_ARM_DMG);
  }

  // jets
  if ((soldier.control.jetpack) && (soldier.jetscount > 0))
  {
    auto it = visible.find(GOSTEK_LEFT_FOOT);
    if (it != visible.end())
    {
      visible.erase(it);
    }
    it = visible.find(GOSTEK_RIGHT_FOOT);
    if (it != visible.end())
    {
      visible.erase(it);
    }
    visible.emplace(GOSTEK_LEFT_JETFOOT);
    visible.emplace(GOSTEK_RIGHT_JETFOOT);
  }

  // vest
  if (soldier.vest > 0)
  {
    include(visible, GOSTEK_VEST);
  }

  // grenades
  if (soldier.tertiaryweapon.num == fraggrenade_num)
  {
    index = GOSTEK_FRAG_GRENADE1;
  }
  else
  {
    index = GOSTEK_CLUSTER_GRENADE1;
  }

  std::int32_t n =
    soldier.tertiaryweapon.ammocount - ord(soldier.bodyanimation.id == AnimationType::Throw);

  for (i = 0; i <= min(5, n) - 1; i++)
  {
    include(visible, index + i);
  }

  // chain
  switch (soldier.player->chain)
  {
  case 1:
    visible.emplace(GOSTEK_SILVER_LCHAIN);
    visible.emplace(GOSTEK_SILVER_RCHAIN);
    visible.emplace(GOSTEK_SILVER_PENDANT);
    break;
  case 2:
    visible.emplace(GOSTEK_GOLDEN_LCHAIN);
    visible.emplace(GOSTEK_GOLDEN_RCHAIN);
    visible.emplace(GOSTEK_GOLDEN_PENDANT);
    break;
  }

  // cygar
  if ((soldier.hascigar == 5) || (soldier.hascigar == 10))
  {
    include(visible, GOSTEK_CIGAR);
  }

  // head & hair
  if (soldier.deadmeat)
  {
    auto it = visible.find(GOSTEK_HEAD);
    if (it != visible.end())
    {
      visible.erase(it);
    }
    it = visible.find(GOSTEK_HEAD_DMG);
    if (it != visible.end())
    {
      visible.erase(it);
    }
    visible.emplace(GOSTEK_HEAD_DEAD);
    visible.emplace(GOSTEK_HEAD_DEAD_DMG);
  }

  if (soldier.weapon.num == bow_num || bow2_num == soldier.weapon.num)
  {
    include(visible, GOSTEK_RAMBO_BADGE);
  }
  else
  {
    bool grabbed = (AnimationType::Wipe == soldier.bodyanimation.id ||
                    AnimationType::TakeOff == soldier.bodyanimation.id) &&
                   (soldier.bodyanimation.currframe > 4);

    if (soldier.wearhelmet == 1)
    {
      switch (soldier.player->headcap)
      {
      case GFX::GOSTEK_HELM:
        if (grabbed)
        {
          include(visible, GOSTEK_GRABBED_HELMET);
        }
        else
        {
          include(visible, GOSTEK_HELMET);
        }
        break;
      case GFX::GOSTEK_KAP:
        if (grabbed)
        {
          include(visible, GOSTEK_GRABBED_HAT);
        }
        else
        {
          include(visible, GOSTEK_HAT);
        }
        break;
      }
    }

    if (grabbed || (soldier.wearhelmet != 1) || (soldier.player->hairstyle == 3))
    {
      switch (soldier.player->hairstyle)
      {
      case 1:
        for (i = 0; i <= 5; i++)
        {
          include(visible, GOSTEK_HAIR_DREADLOCKS + i);
        }
        break;
      case 2:
        include(visible, GOSTEK_HAIR_PUNK);
        break;
      case 3:
        include(visible, GOSTEK_MR_T);
        break;
      case 4:
        include(visible, GOSTEK_HAIR_NORMAL);
        break;
      }
    }
  }

  // secondary weapon (on the back)

  index = weaponnumtoindex(soldier.secondaryweapon.num, GS::GetWeaponSystem().GetGuns());

  if ((index >= eagle) && (index <= flamer))
  {
    include(visible, GOSTEK_SECONDARY_FIRST + index - eagle);
  }

  // primary weapon

  if (soldier.weapon.num == minigun_num)
  {
    include(visible, GOSTEK_PRIMARY_MINIGUN);

    showclip = (soldier.weapon.ammocount > 0) ||
               ((soldier.weapon.ammocount == 0) && (soldier.weapon.reloadtimecount < 65));

    if (showclip)
    {
      include(visible, GOSTEK_PRIMARY_MINIGUN_CLIP);
    }

    if (soldier.fired > 0)
    {
      include(visible, GOSTEK_PRIMARY_MINIGUN_FIRE);
    }
  }
  else if (soldier.weapon.num == bow_num || bow2_num == soldier.weapon.num)
  {
    if (soldier.weapon.ammocount == 0)
    {
      include(visible, GOSTEK_PRIMARY_BOW_ARROW_RELOAD);
    }
    else
    {
      include(visible, GOSTEK_PRIMARY_BOW_ARROW);
    }

    if (soldier.bodyanimation.id == AnimationType::ReloadBow)
    {
      visible.emplace(GOSTEK_PRIMARY_BOW_RELOAD);
      visible.emplace(GOSTEK_PRIMARY_BOW_STRING_RELOAD);
    }
    else
    {
      visible.emplace(GOSTEK_PRIMARY_BOW);
      visible.emplace(GOSTEK_PRIMARY_BOW_STRING);
    }

    if (soldier.fired > 0)
    {
      include(visible, GOSTEK_PRIMARY_BOW_FIRE);
    }
  }
  else if (!soldier.deadmeat)
  {
    index = weaponnumtoindex(soldier.weapon.num, GS::GetWeaponSystem().GetGuns());

    if ((index >= eagle) && (index <= flamer))
    {
      if (index == flamer)
      {
        index = GOSTEK_PRIMARY_FLAMER - GOSTEK_PRIMARY_FIRST;
      }
      else
      {
        index = 3 * (index - eagle);
      }

      include(visible, GOSTEK_PRIMARY_FIRST + index);

      showclip = (soldier.weapon.cliptexturenum > 0) &&
                 ((soldier.weapon.ammocount > 0) ||
                  ((soldier.weapon.ammocount == 0) &&
                   ((soldier.weapon.reloadtimecount < soldier.weapon.clipintime) ||
                    (soldier.weapon.reloadtimecount > soldier.weapon.clipouttime))));

      if (showclip)
      {
        include(visible, GOSTEK_PRIMARY_FIRST + index + 1);
      }

      if (soldier.fired > 0)
      {
        include(visible, GOSTEK_PRIMARY_FIRST + index + 2);
      }
    }
  }

  // draw

  std::int32_t team2offset = 0;

  if (soldier.player->team == team_bravo || team_delta == soldier.player->team)
  {
    team2offset = GFX::GOSTEK_TEAM2_STOPA - GFX::GOSTEK_STOPA;
  }

  if (visible.contains(GOSTEK_HAIR_DREADLOCKS))
  {
    x1 = soldier.skeleton.pos[gosteksprites[GOSTEK_HEAD].p1].x;
    y1 = soldier.skeleton.pos[gosteksprites[GOSTEK_HEAD].p1].y;
    x2 = soldier.skeleton.pos[gosteksprites[GOSTEK_HEAD].p2].x;
    y2 = soldier.skeleton.pos[gosteksprites[GOSTEK_HEAD].p2].y;
    r = atan2(y2 - y1, x2 - x1) - pi / 2;
    m = gfxmat3rot(r);
  }

  for (i = GOSTEK_FIRST; i < GOSTEK_LAST; i++)
  {

    if (const auto &gs = gosteksprites[i]; (visible.contains(i)) && (gs.image != 0))
    {
      std::int32_t tex = gs.image;

      if (gs.team)
      {
        tex = tex + team2offset;
      }

      x1 = soldier.skeleton.pos[gs.p1].x;
      y1 = soldier.skeleton.pos[gs.p1].y;
      x2 = soldier.skeleton.pos[gs.p2].x;
      y2 = soldier.skeleton.pos[gs.p2].y;
      r = atan2(y2 - y1, x2 - x1);
      float cx = gs.cx;
      float cy = gs.cy;
      float sx = 1;
      float sy = 1;

      if (soldier.direction != 1)
      {
        if (gs.flip)
        {
          cy = 1 - gs.cy;
          tex = tex + 1;
        }
        else
        {
          sy = -1;
        }
      }

      cx = cx * textures[tex]->width * textures[tex]->scale;
      cy = cy * textures[tex]->height * textures[tex]->scale;
      color[gs.color].color.a = alpha[gs.alpha];

      if ((i >= GOSTEK_HAIR_DREADLOCK1) && (i <= GOSTEK_HAIR_DREADLOCK5))
      {
        tvector2 v = gfxmat3mul(m, -cy * soldier.direction, cx);
        x1 = x1 + v.x;
        y1 = y1 + v.y;
        cx = 0;
        cy = 0.5 * textures[tex]->height * textures[tex]->scale;
        sx = 0.75 + (1 - 0.75) / 5 * (i - GOSTEK_HAIR_DREADLOCK1);
      }
      else if (gs.flex > 0)
      {
        sx = min(1.5, sqrt(sqr(x2 - x1) + sqr(y2 - y1)) / gs.flex);
      }

      drawgosteksprite(textures[tex], x1, y1 + 1, sx, sy, cx, cy, r, color[gs.color]);
    }
  }
}

// TESTS
#include <doctest.h>

struct GostekGraphicsFixture
{
public:
  GostekGraphicsFixture() = default;
  template <typename T>
  void VerifyGostSkprites(const T &gosteksprites);
  void VerifyGostekBase(const tgostekspriteset &gostekbase);
};

TEST_SUITE("GostekGraphics")
{
  TEST_CASE_FIXTURE(GostekGraphicsFixture, "Load defaults clears gostekbase")
  {
    VerifyGostSkprites(sGostekSpritesDefaults);
    loaddefaults();
    VerifyGostekBase(gostekbase);
    VerifyGostSkprites(gosteksprites);
    loaddefaults();
    VerifyGostekBase(gostekbase);
    VerifyGostSkprites(gosteksprites);
  }
}
// NOLINTBEGIN
template <typename T>
void GostekGraphicsFixture::VerifyGostSkprites(const T &gosteksprites)
{
  CHECK_EQ(gosteksprites[0].cx, 0.300000f);
  CHECK_EQ(gosteksprites[0].image, 0);
  CHECK_EQ(gosteksprites[0].p1, 5);
  CHECK_EQ(gosteksprites[0].p2, 10);
  CHECK_EQ(gosteksprites[0].id, "Secondary_Deagles");
  CHECK_EQ(gosteksprites[0].cy, 0.500000f);
  CHECK_EQ(gosteksprites[0].flex, 0.000000f);
  CHECK_EQ(gosteksprites[0].flip, true);
  CHECK_EQ(gosteksprites[0].team, false);
  CHECK_EQ(gosteksprites[0].color, 0);
  CHECK_EQ(gosteksprites[0].alpha, 0);
  CHECK_EQ(gosteksprites[1].cx, 0.300000f);
  CHECK_EQ(gosteksprites[1].image, 148);
  CHECK_EQ(gosteksprites[1].p1, 5);
  CHECK_EQ(gosteksprites[1].p2, 10);
  CHECK_EQ(gosteksprites[1].id, "Secondary_Mp5");
  CHECK_EQ(gosteksprites[1].cy, 0.300000f);
  CHECK_EQ(gosteksprites[1].flex, 0.000000f);
  CHECK_EQ(gosteksprites[1].flip, true);
  CHECK_EQ(gosteksprites[1].team, false);
  CHECK_EQ(gosteksprites[1].color, 0);
  CHECK_EQ(gosteksprites[1].alpha, 0);
  CHECK_EQ(gosteksprites[2].cx, 0.300000f);
  CHECK_EQ(gosteksprites[2].image, 129);
  CHECK_EQ(gosteksprites[2].p1, 5);
  CHECK_EQ(gosteksprites[2].p2, 10);
  CHECK_EQ(gosteksprites[2].id, "Secondary_Ak74");
  CHECK_EQ(gosteksprites[2].cy, 0.250000f);
  CHECK_EQ(gosteksprites[2].flex, 0.000000f);
  CHECK_EQ(gosteksprites[2].flip, true);
  CHECK_EQ(gosteksprites[2].team, false);
  CHECK_EQ(gosteksprites[2].color, 0);
  CHECK_EQ(gosteksprites[2].alpha, 0);
  CHECK_EQ(gosteksprites[3].cx, 0.300000f);
  CHECK_EQ(gosteksprites[3].image, 176);
  CHECK_EQ(gosteksprites[3].p1, 5);
  CHECK_EQ(gosteksprites[3].p2, 10);
  CHECK_EQ(gosteksprites[3].id, "Secondary_Steyr");
  CHECK_EQ(gosteksprites[3].cy, 0.500000f);
  CHECK_EQ(gosteksprites[3].flex, 0.000000f);
  CHECK_EQ(gosteksprites[3].flip, true);
  CHECK_EQ(gosteksprites[3].team, false);
  CHECK_EQ(gosteksprites[3].color, 0);
  CHECK_EQ(gosteksprites[3].alpha, 0);
  CHECK_EQ(gosteksprites[4].cx, 0.300000f);
  CHECK_EQ(gosteksprites[4].image, 155);
  CHECK_EQ(gosteksprites[4].p1, 5);
  CHECK_EQ(gosteksprites[4].p2, 10);
  CHECK_EQ(gosteksprites[4].id, "Secondary_Spas");
  CHECK_EQ(gosteksprites[4].cy, 0.300000f);
  CHECK_EQ(gosteksprites[4].flex, 0.000000f);
  CHECK_EQ(gosteksprites[4].flip, true);
  CHECK_EQ(gosteksprites[4].team, false);
  CHECK_EQ(gosteksprites[4].color, 0);
  CHECK_EQ(gosteksprites[4].alpha, 0);
  CHECK_EQ(gosteksprites[5].cx, 0.300000f);
  CHECK_EQ(gosteksprites[5].image, 143);
  CHECK_EQ(gosteksprites[5].p1, 5);
  CHECK_EQ(gosteksprites[5].p2, 10);
  CHECK_EQ(gosteksprites[5].id, "Secondary_Ruger");
  CHECK_EQ(gosteksprites[5].cy, 0.300000f);
  CHECK_EQ(gosteksprites[5].flex, 0.000000f);
  CHECK_EQ(gosteksprites[5].flip, true);
  CHECK_EQ(gosteksprites[5].team, false);
  CHECK_EQ(gosteksprites[5].color, 0);
  CHECK_EQ(gosteksprites[5].alpha, 0);
  CHECK_EQ(gosteksprites[6].cx, 0.300000f);
  CHECK_EQ(gosteksprites[6].image, 160);
  CHECK_EQ(gosteksprites[6].p1, 5);
  CHECK_EQ(gosteksprites[6].p2, 10);
  CHECK_EQ(gosteksprites[6].id, "Secondary_M79");
  CHECK_EQ(gosteksprites[6].cy, 0.350000f);
  CHECK_EQ(gosteksprites[6].flex, 0.000000f);
  CHECK_EQ(gosteksprites[6].flip, true);
  CHECK_EQ(gosteksprites[6].team, false);
  CHECK_EQ(gosteksprites[6].color, 0);
  CHECK_EQ(gosteksprites[6].alpha, 0);
  CHECK_EQ(gosteksprites[7].cx, 0.300000f);
  CHECK_EQ(gosteksprites[7].image, 183);
  CHECK_EQ(gosteksprites[7].p1, 5);
  CHECK_EQ(gosteksprites[7].p2, 10);
  CHECK_EQ(gosteksprites[7].id, "Secondary_Barrett");
  CHECK_EQ(gosteksprites[7].cy, 0.350000f);
  CHECK_EQ(gosteksprites[7].flex, 0.000000f);
  CHECK_EQ(gosteksprites[7].flip, true);
  CHECK_EQ(gosteksprites[7].team, false);
  CHECK_EQ(gosteksprites[7].color, 0);
  CHECK_EQ(gosteksprites[7].alpha, 0);
  CHECK_EQ(gosteksprites[8].cx, 0.300000f);
  CHECK_EQ(gosteksprites[8].image, 136);
  CHECK_EQ(gosteksprites[8].p1, 5);
  CHECK_EQ(gosteksprites[8].p2, 10);
  CHECK_EQ(gosteksprites[8].id, "Secondary_Minimi");
  CHECK_EQ(gosteksprites[8].cy, 0.350000f);
  CHECK_EQ(gosteksprites[8].flex, 0.000000f);
  CHECK_EQ(gosteksprites[8].flip, true);
  CHECK_EQ(gosteksprites[8].team, false);
  CHECK_EQ(gosteksprites[8].color, 0);
  CHECK_EQ(gosteksprites[8].alpha, 0);
  CHECK_EQ(gosteksprites[9].cx, 0.200000f);
  CHECK_EQ(gosteksprites[9].image, 190);
  CHECK_EQ(gosteksprites[9].p1, 5);
  CHECK_EQ(gosteksprites[9].p2, 10);
  CHECK_EQ(gosteksprites[9].id, "Secondary_Minigun");
  CHECK_EQ(gosteksprites[9].cy, 0.500000f);
  CHECK_EQ(gosteksprites[9].flex, 0.000000f);
  CHECK_EQ(gosteksprites[9].flip, true);
  CHECK_EQ(gosteksprites[9].team, false);
  CHECK_EQ(gosteksprites[9].color, 0);
  CHECK_EQ(gosteksprites[9].alpha, 0);
  CHECK_EQ(gosteksprites[10].cx, 0.300000f);
  CHECK_EQ(gosteksprites[10].image, 0);
  CHECK_EQ(gosteksprites[10].p1, 5);
  CHECK_EQ(gosteksprites[10].p2, 10);
  CHECK_EQ(gosteksprites[10].id, "Secondary_Socom");
  CHECK_EQ(gosteksprites[10].cy, 0.500000f);
  CHECK_EQ(gosteksprites[10].flex, 0.000000f);
  CHECK_EQ(gosteksprites[10].flip, true);
  CHECK_EQ(gosteksprites[10].team, false);
  CHECK_EQ(gosteksprites[10].color, 0);
  CHECK_EQ(gosteksprites[10].alpha, 0);
  CHECK_EQ(gosteksprites[11].cx, 0.300000f);
  CHECK_EQ(gosteksprites[11].image, 0);
  CHECK_EQ(gosteksprites[11].p1, 5);
  CHECK_EQ(gosteksprites[11].p2, 10);
  CHECK_EQ(gosteksprites[11].id, "Secondary_Knife");
  CHECK_EQ(gosteksprites[11].cy, 0.500000f);
  CHECK_EQ(gosteksprites[11].flex, 0.000000f);
  CHECK_EQ(gosteksprites[11].flip, true);
  CHECK_EQ(gosteksprites[11].team, false);
  CHECK_EQ(gosteksprites[11].color, 0);
  CHECK_EQ(gosteksprites[11].alpha, 0);
  CHECK_EQ(gosteksprites[12].cx, 0.250000f);
  CHECK_EQ(gosteksprites[12].image, 218);
  CHECK_EQ(gosteksprites[12].p1, 5);
  CHECK_EQ(gosteksprites[12].p2, 10);
  CHECK_EQ(gosteksprites[12].id, "Secondary_Chainsaw");
  CHECK_EQ(gosteksprites[12].cy, 0.500000f);
  CHECK_EQ(gosteksprites[12].flex, 0.000000f);
  CHECK_EQ(gosteksprites[12].flip, true);
  CHECK_EQ(gosteksprites[12].team, false);
  CHECK_EQ(gosteksprites[12].color, 0);
  CHECK_EQ(gosteksprites[12].alpha, 0);
  CHECK_EQ(gosteksprites[13].cx, 0.300000f);
  CHECK_EQ(gosteksprites[13].image, 221);
  CHECK_EQ(gosteksprites[13].p1, 5);
  CHECK_EQ(gosteksprites[13].p2, 10);
  CHECK_EQ(gosteksprites[13].id, "Secondary_Law");
  CHECK_EQ(gosteksprites[13].cy, 0.450000f);
  CHECK_EQ(gosteksprites[13].flex, 0.000000f);
  CHECK_EQ(gosteksprites[13].flip, true);
  CHECK_EQ(gosteksprites[13].team, false);
  CHECK_EQ(gosteksprites[13].color, 0);
  CHECK_EQ(gosteksprites[13].alpha, 0);
  CHECK_EQ(gosteksprites[14].cx, 0.300000f);
  CHECK_EQ(gosteksprites[14].image, 0);
  CHECK_EQ(gosteksprites[14].p1, 5);
  CHECK_EQ(gosteksprites[14].p2, 10);
  CHECK_EQ(gosteksprites[14].id, "Secondary_Flamebow");
  CHECK_EQ(gosteksprites[14].cy, 0.500000f);
  CHECK_EQ(gosteksprites[14].flex, 0.000000f);
  CHECK_EQ(gosteksprites[14].flip, true);
  CHECK_EQ(gosteksprites[14].team, false);
  CHECK_EQ(gosteksprites[14].color, 0);
  CHECK_EQ(gosteksprites[14].alpha, 0);
  CHECK_EQ(gosteksprites[15].cx, 0.300000f);
  CHECK_EQ(gosteksprites[15].image, 0);
  CHECK_EQ(gosteksprites[15].p1, 5);
  CHECK_EQ(gosteksprites[15].p2, 10);
  CHECK_EQ(gosteksprites[15].id, "Secondary_Bow");
  CHECK_EQ(gosteksprites[15].cy, 0.500000f);
  CHECK_EQ(gosteksprites[15].flex, 0.000000f);
  CHECK_EQ(gosteksprites[15].flip, true);
  CHECK_EQ(gosteksprites[15].team, false);
  CHECK_EQ(gosteksprites[15].color, 0);
  CHECK_EQ(gosteksprites[15].alpha, 0);
  CHECK_EQ(gosteksprites[16].cx, 0.300000f);
  CHECK_EQ(gosteksprites[16].image, 213);
  CHECK_EQ(gosteksprites[16].p1, 5);
  CHECK_EQ(gosteksprites[16].p2, 10);
  CHECK_EQ(gosteksprites[16].id, "Secondary_Flamer");
  CHECK_EQ(gosteksprites[16].cy, 0.300000f);
  CHECK_EQ(gosteksprites[16].flex, 0.000000f);
  CHECK_EQ(gosteksprites[16].flip, true);
  CHECK_EQ(gosteksprites[16].team, false);
  CHECK_EQ(gosteksprites[16].color, 0);
  CHECK_EQ(gosteksprites[16].alpha, 0);
  CHECK_EQ(gosteksprites[17].cx, 0.200000f);
  CHECK_EQ(gosteksprites[17].image, 5);
  CHECK_EQ(gosteksprites[17].p1, 6);
  CHECK_EQ(gosteksprites[17].p2, 3);
  CHECK_EQ(gosteksprites[17].id, "Left_Thigh");
  CHECK_EQ(gosteksprites[17].cy, 0.500000f);
  CHECK_EQ(gosteksprites[17].flex, 5.000000f);
  CHECK_EQ(gosteksprites[17].flip, true);
  CHECK_EQ(gosteksprites[17].team, true);
  CHECK_EQ(gosteksprites[17].color, 2);
  CHECK_EQ(gosteksprites[17].alpha, 0);
  CHECK_EQ(gosteksprites[18].cx, 0.200000f);
  CHECK_EQ(gosteksprites[18].image, 54);
  CHECK_EQ(gosteksprites[18].p1, 6);
  CHECK_EQ(gosteksprites[18].p2, 3);
  CHECK_EQ(gosteksprites[18].id, "Left_Thigh_Dmg");
  CHECK_EQ(gosteksprites[18].cy, 0.500000f);
  CHECK_EQ(gosteksprites[18].flex, 5.000000f);
  CHECK_EQ(gosteksprites[18].flip, true);
  CHECK_EQ(gosteksprites[18].team, true);
  CHECK_EQ(gosteksprites[18].color, 0);
  CHECK_EQ(gosteksprites[18].alpha, 1);
  CHECK_EQ(gosteksprites[19].cx, 0.350000f);
  CHECK_EQ(gosteksprites[19].image, 1);
  CHECK_EQ(gosteksprites[19].p1, 2);
  CHECK_EQ(gosteksprites[19].p2, 18);
  CHECK_EQ(gosteksprites[19].id, "Left_Foot");
  CHECK_EQ(gosteksprites[19].cy, 0.350000f);
  CHECK_EQ(gosteksprites[19].flex, 0.000000f);
  CHECK_EQ(gosteksprites[19].flip, true);
  CHECK_EQ(gosteksprites[19].team, true);
  CHECK_EQ(gosteksprites[19].color, 0);
  CHECK_EQ(gosteksprites[19].alpha, 0);
  CHECK_EQ(gosteksprites[20].cx, 0.350000f);
  CHECK_EQ(gosteksprites[20].image, 58);
  CHECK_EQ(gosteksprites[20].p1, 2);
  CHECK_EQ(gosteksprites[20].p2, 18);
  CHECK_EQ(gosteksprites[20].id, "Left_Jetfoot");
  CHECK_EQ(gosteksprites[20].cy, 0.350000f);
  CHECK_EQ(gosteksprites[20].flex, 0.000000f);
  CHECK_EQ(gosteksprites[20].flip, true);
  CHECK_EQ(gosteksprites[20].team, true);
  CHECK_EQ(gosteksprites[20].color, 0);
  CHECK_EQ(gosteksprites[20].alpha, 0);
  CHECK_EQ(gosteksprites[21].cx, 0.150000f);
  CHECK_EQ(gosteksprites[21].image, 3);
  CHECK_EQ(gosteksprites[21].p1, 3);
  CHECK_EQ(gosteksprites[21].p2, 2);
  CHECK_EQ(gosteksprites[21].id, "Left_Lowerleg");
  CHECK_EQ(gosteksprites[21].cy, 0.550000f);
  CHECK_EQ(gosteksprites[21].flex, 0.000000f);
  CHECK_EQ(gosteksprites[21].flip, true);
  CHECK_EQ(gosteksprites[21].team, true);
  CHECK_EQ(gosteksprites[21].color, 2);
  CHECK_EQ(gosteksprites[21].alpha, 0);
  CHECK_EQ(gosteksprites[22].cx, 0.150000f);
  CHECK_EQ(gosteksprites[22].image, 48);
  CHECK_EQ(gosteksprites[22].p1, 3);
  CHECK_EQ(gosteksprites[22].p2, 2);
  CHECK_EQ(gosteksprites[22].id, "Left_Lowerleg_Dmg");
  CHECK_EQ(gosteksprites[22].cy, 0.550000f);
  CHECK_EQ(gosteksprites[22].flex, 0.000000f);
  CHECK_EQ(gosteksprites[22].flip, true);
  CHECK_EQ(gosteksprites[22].team, true);
  CHECK_EQ(gosteksprites[22].color, 0);
  CHECK_EQ(gosteksprites[22].alpha, 1);
  CHECK_EQ(gosteksprites[23].cx, 0.000000f);
  CHECK_EQ(gosteksprites[23].image, 13);
  CHECK_EQ(gosteksprites[23].p1, 11);
  CHECK_EQ(gosteksprites[23].p2, 14);
  CHECK_EQ(gosteksprites[23].id, "Left_Arm");
  CHECK_EQ(gosteksprites[23].cy, 0.500000f);
  CHECK_EQ(gosteksprites[23].flex, 0.000000f);
  CHECK_EQ(gosteksprites[23].flip, true);
  CHECK_EQ(gosteksprites[23].team, true);
  CHECK_EQ(gosteksprites[23].color, 1);
  CHECK_EQ(gosteksprites[23].alpha, 0);
  CHECK_EQ(gosteksprites[24].cx, 0.000000f);
  CHECK_EQ(gosteksprites[24].image, 50);
  CHECK_EQ(gosteksprites[24].p1, 11);
  CHECK_EQ(gosteksprites[24].p2, 14);
  CHECK_EQ(gosteksprites[24].id, "Left_Arm_Dmg");
  CHECK_EQ(gosteksprites[24].cy, 0.500000f);
  CHECK_EQ(gosteksprites[24].flex, 0.000000f);
  CHECK_EQ(gosteksprites[24].flip, true);
  CHECK_EQ(gosteksprites[24].team, true);
  CHECK_EQ(gosteksprites[24].color, 0);
  CHECK_EQ(gosteksprites[24].alpha, 1);
  CHECK_EQ(gosteksprites[25].cx, 0.000000f);
  CHECK_EQ(gosteksprites[25].image, 15);
  CHECK_EQ(gosteksprites[25].p1, 14);
  CHECK_EQ(gosteksprites[25].p2, 15);
  CHECK_EQ(gosteksprites[25].id, "Left_Forearm");
  CHECK_EQ(gosteksprites[25].cy, 0.500000f);
  CHECK_EQ(gosteksprites[25].flex, 5.000000f);
  CHECK_EQ(gosteksprites[25].flip, false);
  CHECK_EQ(gosteksprites[25].team, true);
  CHECK_EQ(gosteksprites[25].color, 1);
  CHECK_EQ(gosteksprites[25].alpha, 0);
  CHECK_EQ(gosteksprites[26].cx, 0.000000f);
  CHECK_EQ(gosteksprites[26].image, 52);
  CHECK_EQ(gosteksprites[26].p1, 14);
  CHECK_EQ(gosteksprites[26].p2, 15);
  CHECK_EQ(gosteksprites[26].id, "Left_Forearm_Dmg");
  CHECK_EQ(gosteksprites[26].cy, 0.500000f);
  CHECK_EQ(gosteksprites[26].flex, 5.000000f);
  CHECK_EQ(gosteksprites[26].flip, true);
  CHECK_EQ(gosteksprites[26].team, true);
  CHECK_EQ(gosteksprites[26].color, 0);
  CHECK_EQ(gosteksprites[26].alpha, 1);
  CHECK_EQ(gosteksprites[27].cx, 0.000000f);
  CHECK_EQ(gosteksprites[27].image, 17);
  CHECK_EQ(gosteksprites[27].p1, 15);
  CHECK_EQ(gosteksprites[27].p2, 19);
  CHECK_EQ(gosteksprites[27].id, "Left_Hand");
  CHECK_EQ(gosteksprites[27].cy, 0.400000f);
  CHECK_EQ(gosteksprites[27].flex, 0.000000f);
  CHECK_EQ(gosteksprites[27].flip, true);
  CHECK_EQ(gosteksprites[27].team, true);
  CHECK_EQ(gosteksprites[27].color, 3);
  CHECK_EQ(gosteksprites[27].alpha, 0);
  CHECK_EQ(gosteksprites[28].cx, 0.000000f);
  CHECK_EQ(gosteksprites[28].image, 20);
  CHECK_EQ(gosteksprites[28].p1, 15);
  CHECK_EQ(gosteksprites[28].p2, 19);
  CHECK_EQ(gosteksprites[28].id, "Grabbed_Helmet");
  CHECK_EQ(gosteksprites[28].cy, 0.500000f);
  CHECK_EQ(gosteksprites[28].flex, 0.000000f);
  CHECK_EQ(gosteksprites[28].flip, true);
  CHECK_EQ(gosteksprites[28].team, true);
  CHECK_EQ(gosteksprites[28].color, 1);
  CHECK_EQ(gosteksprites[28].alpha, 0);
  CHECK_EQ(gosteksprites[29].cx, 0.100000f);
  CHECK_EQ(gosteksprites[29].image, 31);
  CHECK_EQ(gosteksprites[29].p1, 15);
  CHECK_EQ(gosteksprites[29].p2, 19);
  CHECK_EQ(gosteksprites[29].id, "Grabbed_Hat");
  CHECK_EQ(gosteksprites[29].cy, 0.400000f);
  CHECK_EQ(gosteksprites[29].flex, 0.000000f);
  CHECK_EQ(gosteksprites[29].flip, true);
  CHECK_EQ(gosteksprites[29].team, true);
  CHECK_EQ(gosteksprites[29].color, 1);
  CHECK_EQ(gosteksprites[29].alpha, 0);
  CHECK_EQ(gosteksprites[30].cx, 0.200000f);
  CHECK_EQ(gosteksprites[30].image, 5);
  CHECK_EQ(gosteksprites[30].p1, 5);
  CHECK_EQ(gosteksprites[30].p2, 4);
  CHECK_EQ(gosteksprites[30].id, "Right_Thigh");
  CHECK_EQ(gosteksprites[30].cy, 0.650000f);
  CHECK_EQ(gosteksprites[30].flex, 5.000000f);
  CHECK_EQ(gosteksprites[30].flip, true);
  CHECK_EQ(gosteksprites[30].team, true);
  CHECK_EQ(gosteksprites[30].color, 2);
  CHECK_EQ(gosteksprites[30].alpha, 0);
  CHECK_EQ(gosteksprites[31].cx, 0.200000f);
  CHECK_EQ(gosteksprites[31].image, 54);
  CHECK_EQ(gosteksprites[31].p1, 5);
  CHECK_EQ(gosteksprites[31].p2, 4);
  CHECK_EQ(gosteksprites[31].id, "Right_Thigh_Dmg");
  CHECK_EQ(gosteksprites[31].cy, 0.650000f);
  CHECK_EQ(gosteksprites[31].flex, 5.000000f);
  CHECK_EQ(gosteksprites[31].flip, true);
  CHECK_EQ(gosteksprites[31].team, true);
  CHECK_EQ(gosteksprites[31].color, 0);
  CHECK_EQ(gosteksprites[31].alpha, 1);
  CHECK_EQ(gosteksprites[32].cx, 0.350000f);
  CHECK_EQ(gosteksprites[32].image, 1);
  CHECK_EQ(gosteksprites[32].p1, 1);
  CHECK_EQ(gosteksprites[32].p2, 17);
  CHECK_EQ(gosteksprites[32].id, "Right_Foot");
  CHECK_EQ(gosteksprites[32].cy, 0.350000f);
  CHECK_EQ(gosteksprites[32].flex, 0.000000f);
  CHECK_EQ(gosteksprites[32].flip, true);
  CHECK_EQ(gosteksprites[32].team, true);
  CHECK_EQ(gosteksprites[32].color, 0);
  CHECK_EQ(gosteksprites[32].alpha, 0);
  CHECK_EQ(gosteksprites[33].cx, 0.350000f);
  CHECK_EQ(gosteksprites[33].image, 58);
  CHECK_EQ(gosteksprites[33].p1, 1);
  CHECK_EQ(gosteksprites[33].p2, 17);
  CHECK_EQ(gosteksprites[33].id, "Right_Jetfoot");
  CHECK_EQ(gosteksprites[33].cy, 0.350000f);
  CHECK_EQ(gosteksprites[33].flex, 0.000000f);
  CHECK_EQ(gosteksprites[33].flip, true);
  CHECK_EQ(gosteksprites[33].team, true);
  CHECK_EQ(gosteksprites[33].color, 0);
  CHECK_EQ(gosteksprites[33].alpha, 0);
  CHECK_EQ(gosteksprites[34].cx, 0.150000f);
  CHECK_EQ(gosteksprites[34].image, 3);
  CHECK_EQ(gosteksprites[34].p1, 4);
  CHECK_EQ(gosteksprites[34].p2, 1);
  CHECK_EQ(gosteksprites[34].id, "Right_Lowerleg");
  CHECK_EQ(gosteksprites[34].cy, 0.550000f);
  CHECK_EQ(gosteksprites[34].flex, 0.000000f);
  CHECK_EQ(gosteksprites[34].flip, true);
  CHECK_EQ(gosteksprites[34].team, true);
  CHECK_EQ(gosteksprites[34].color, 2);
  CHECK_EQ(gosteksprites[34].alpha, 0);
  CHECK_EQ(gosteksprites[35].cx, 0.150000f);
  CHECK_EQ(gosteksprites[35].image, 48);
  CHECK_EQ(gosteksprites[35].p1, 4);
  CHECK_EQ(gosteksprites[35].p2, 1);
  CHECK_EQ(gosteksprites[35].id, "Right_Lowerleg_Dmg");
  CHECK_EQ(gosteksprites[35].cy, 0.550000f);
  CHECK_EQ(gosteksprites[35].flex, 0.000000f);
  CHECK_EQ(gosteksprites[35].flip, true);
  CHECK_EQ(gosteksprites[35].team, true);
  CHECK_EQ(gosteksprites[35].color, 0);
  CHECK_EQ(gosteksprites[35].alpha, 1);
  CHECK_EQ(gosteksprites[36].cx, 0.100000f);
  CHECK_EQ(gosteksprites[36].image, 9);
  CHECK_EQ(gosteksprites[36].p1, 10);
  CHECK_EQ(gosteksprites[36].p2, 11);
  CHECK_EQ(gosteksprites[36].id, "Chest");
  CHECK_EQ(gosteksprites[36].cy, 0.300000f);
  CHECK_EQ(gosteksprites[36].flex, 0.000000f);
  CHECK_EQ(gosteksprites[36].flip, true);
  CHECK_EQ(gosteksprites[36].team, true);
  CHECK_EQ(gosteksprites[36].color, 1);
  CHECK_EQ(gosteksprites[36].alpha, 0);
  CHECK_EQ(gosteksprites[37].cx, 0.100000f);
  CHECK_EQ(gosteksprites[37].image, 42);
  CHECK_EQ(gosteksprites[37].p1, 10);
  CHECK_EQ(gosteksprites[37].p2, 11);
  CHECK_EQ(gosteksprites[37].id, "Vest");
  CHECK_EQ(gosteksprites[37].cy, 0.300000f);
  CHECK_EQ(gosteksprites[37].flex, 0.000000f);
  CHECK_EQ(gosteksprites[37].flip, true);
  CHECK_EQ(gosteksprites[37].team, true);
  CHECK_EQ(gosteksprites[37].color, 0);
  CHECK_EQ(gosteksprites[37].alpha, 0);
  CHECK_EQ(gosteksprites[38].cx, 0.100000f);
  CHECK_EQ(gosteksprites[38].image, 44);
  CHECK_EQ(gosteksprites[38].p1, 10);
  CHECK_EQ(gosteksprites[38].p2, 11);
  CHECK_EQ(gosteksprites[38].id, "Chest_Dmg");
  CHECK_EQ(gosteksprites[38].cy, 0.300000f);
  CHECK_EQ(gosteksprites[38].flex, 0.000000f);
  CHECK_EQ(gosteksprites[38].flip, true);
  CHECK_EQ(gosteksprites[38].team, true);
  CHECK_EQ(gosteksprites[38].color, 0);
  CHECK_EQ(gosteksprites[38].alpha, 1);
  CHECK_EQ(gosteksprites[39].cx, 0.250000f);
  CHECK_EQ(gosteksprites[39].image, 7);
  CHECK_EQ(gosteksprites[39].p1, 5);
  CHECK_EQ(gosteksprites[39].p2, 6);
  CHECK_EQ(gosteksprites[39].id, "Hip");
  CHECK_EQ(gosteksprites[39].cy, 0.600000f);
  CHECK_EQ(gosteksprites[39].flex, 0.000000f);
  CHECK_EQ(gosteksprites[39].flip, true);
  CHECK_EQ(gosteksprites[39].team, true);
  CHECK_EQ(gosteksprites[39].color, 1);
  CHECK_EQ(gosteksprites[39].alpha, 0);
  CHECK_EQ(gosteksprites[40].cx, 0.250000f);
  CHECK_EQ(gosteksprites[40].image, 56);
  CHECK_EQ(gosteksprites[40].p1, 5);
  CHECK_EQ(gosteksprites[40].p2, 6);
  CHECK_EQ(gosteksprites[40].id, "Hip_Dmg");
  CHECK_EQ(gosteksprites[40].cy, 0.600000f);
  CHECK_EQ(gosteksprites[40].flex, 0.000000f);
  CHECK_EQ(gosteksprites[40].flip, true);
  CHECK_EQ(gosteksprites[40].team, true);
  CHECK_EQ(gosteksprites[40].color, 0);
  CHECK_EQ(gosteksprites[40].alpha, 1);
  CHECK_EQ(gosteksprites[41].cx, 0.000000f);
  CHECK_EQ(gosteksprites[41].image, 11);
  CHECK_EQ(gosteksprites[41].p1, 9);
  CHECK_EQ(gosteksprites[41].p2, 12);
  CHECK_EQ(gosteksprites[41].id, "Head");
  CHECK_EQ(gosteksprites[41].cy, 0.500000f);
  CHECK_EQ(gosteksprites[41].flex, 0.000000f);
  CHECK_EQ(gosteksprites[41].flip, true);
  CHECK_EQ(gosteksprites[41].team, true);
  CHECK_EQ(gosteksprites[41].color, 3);
  CHECK_EQ(gosteksprites[41].alpha, 0);
  CHECK_EQ(gosteksprites[42].cx, 0.000000f);
  CHECK_EQ(gosteksprites[42].image, 46);
  CHECK_EQ(gosteksprites[42].p1, 9);
  CHECK_EQ(gosteksprites[42].p2, 12);
  CHECK_EQ(gosteksprites[42].id, "Head_Dmg");
  CHECK_EQ(gosteksprites[42].cy, 0.500000f);
  CHECK_EQ(gosteksprites[42].flex, 0.000000f);
  CHECK_EQ(gosteksprites[42].flip, true);
  CHECK_EQ(gosteksprites[42].team, true);
  CHECK_EQ(gosteksprites[42].color, 6);
  CHECK_EQ(gosteksprites[42].alpha, 1);
  CHECK_EQ(gosteksprites[43].cx, 0.500000f);
  CHECK_EQ(gosteksprites[43].image, 11);
  CHECK_EQ(gosteksprites[43].p1, 9);
  CHECK_EQ(gosteksprites[43].p2, 12);
  CHECK_EQ(gosteksprites[43].id, "Head_Dead");
  CHECK_EQ(gosteksprites[43].cy, 0.500000f);
  CHECK_EQ(gosteksprites[43].flex, 0.000000f);
  CHECK_EQ(gosteksprites[43].flip, true);
  CHECK_EQ(gosteksprites[43].team, true);
  CHECK_EQ(gosteksprites[43].color, 3);
  CHECK_EQ(gosteksprites[43].alpha, 0);
  CHECK_EQ(gosteksprites[44].cx, 0.500000f);
  CHECK_EQ(gosteksprites[44].image, 46);
  CHECK_EQ(gosteksprites[44].p1, 9);
  CHECK_EQ(gosteksprites[44].p2, 12);
  CHECK_EQ(gosteksprites[44].id, "Head_Dead_Dmg");
  CHECK_EQ(gosteksprites[44].cy, 0.500000f);
  CHECK_EQ(gosteksprites[44].flex, 0.000000f);
  CHECK_EQ(gosteksprites[44].flip, true);
  CHECK_EQ(gosteksprites[44].team, true);
  CHECK_EQ(gosteksprites[44].color, 6);
  CHECK_EQ(gosteksprites[44].alpha, 1);
  CHECK_EQ(gosteksprites[45].cx, 0.000000f);
  CHECK_EQ(gosteksprites[45].image, 38);
  CHECK_EQ(gosteksprites[45].p1, 9);
  CHECK_EQ(gosteksprites[45].p2, 12);
  CHECK_EQ(gosteksprites[45].id, "Mr_T");
  CHECK_EQ(gosteksprites[45].cy, 0.500000f);
  CHECK_EQ(gosteksprites[45].flex, 0.000000f);
  CHECK_EQ(gosteksprites[45].flip, true);
  CHECK_EQ(gosteksprites[45].team, true);
  CHECK_EQ(gosteksprites[45].color, 4);
  CHECK_EQ(gosteksprites[45].alpha, 0);
  CHECK_EQ(gosteksprites[46].cx, 0.000000f);
  CHECK_EQ(gosteksprites[46].image, 20);
  CHECK_EQ(gosteksprites[46].p1, 9);
  CHECK_EQ(gosteksprites[46].p2, 12);
  CHECK_EQ(gosteksprites[46].id, "Helmet");
  CHECK_EQ(gosteksprites[46].cy, 0.500000f);
  CHECK_EQ(gosteksprites[46].flex, 0.000000f);
  CHECK_EQ(gosteksprites[46].flip, true);
  CHECK_EQ(gosteksprites[46].team, true);
  CHECK_EQ(gosteksprites[46].color, 1);
  CHECK_EQ(gosteksprites[46].alpha, 0);
  CHECK_EQ(gosteksprites[47].cx, 0.000000f);
  CHECK_EQ(gosteksprites[47].image, 31);
  CHECK_EQ(gosteksprites[47].p1, 9);
  CHECK_EQ(gosteksprites[47].p2, 12);
  CHECK_EQ(gosteksprites[47].id, "Hat");
  CHECK_EQ(gosteksprites[47].cy, 0.500000f);
  CHECK_EQ(gosteksprites[47].flex, 0.000000f);
  CHECK_EQ(gosteksprites[47].flip, true);
  CHECK_EQ(gosteksprites[47].team, true);
  CHECK_EQ(gosteksprites[47].color, 1);
  CHECK_EQ(gosteksprites[47].alpha, 0);
  CHECK_EQ(gosteksprites[48].cx, 0.000000f);
  CHECK_EQ(gosteksprites[48].image, 22);
  CHECK_EQ(gosteksprites[48].p1, 9);
  CHECK_EQ(gosteksprites[48].p2, 12);
  CHECK_EQ(gosteksprites[48].id, "Rambo_Badge");
  CHECK_EQ(gosteksprites[48].cy, 0.500000f);
  CHECK_EQ(gosteksprites[48].flex, 0.000000f);
  CHECK_EQ(gosteksprites[48].flip, true);
  CHECK_EQ(gosteksprites[48].team, true);
  CHECK_EQ(gosteksprites[48].color, 0);
  CHECK_EQ(gosteksprites[48].alpha, 0);
  CHECK_EQ(gosteksprites[49].cx, 0.000000f);
  CHECK_EQ(gosteksprites[49].image, 34);
  CHECK_EQ(gosteksprites[49].p1, 9);
  CHECK_EQ(gosteksprites[49].p2, 12);
  CHECK_EQ(gosteksprites[49].id, "Hair_Dreadlocks");
  CHECK_EQ(gosteksprites[49].cy, 0.500000f);
  CHECK_EQ(gosteksprites[49].flex, 0.000000f);
  CHECK_EQ(gosteksprites[49].flip, true);
  CHECK_EQ(gosteksprites[49].team, true);
  CHECK_EQ(gosteksprites[49].color, 4);
  CHECK_EQ(gosteksprites[49].alpha, 0);
  CHECK_EQ(gosteksprites[50].cx, 0.000000f);
  CHECK_EQ(gosteksprites[50].image, 33);
  CHECK_EQ(gosteksprites[50].p1, 23);
  CHECK_EQ(gosteksprites[50].p2, 24);
  CHECK_EQ(gosteksprites[50].id, "Hair_Dreadlock1");
  CHECK_EQ(gosteksprites[50].cy, 1.220000f);
  CHECK_EQ(gosteksprites[50].flex, 0.000000f);
  CHECK_EQ(gosteksprites[50].flip, false);
  CHECK_EQ(gosteksprites[50].team, true);
  CHECK_EQ(gosteksprites[50].color, 4);
  CHECK_EQ(gosteksprites[50].alpha, 0);
  CHECK_EQ(gosteksprites[51].cx, 0.100000f);
  CHECK_EQ(gosteksprites[51].image, 33);
  CHECK_EQ(gosteksprites[51].p1, 23);
  CHECK_EQ(gosteksprites[51].p2, 24);
  CHECK_EQ(gosteksprites[51].id, "Hair_Dreadlock2");
  CHECK_EQ(gosteksprites[51].cy, 0.500000f);
  CHECK_EQ(gosteksprites[51].flex, 0.000000f);
  CHECK_EQ(gosteksprites[51].flip, false);
  CHECK_EQ(gosteksprites[51].team, true);
  CHECK_EQ(gosteksprites[51].color, 4);
  CHECK_EQ(gosteksprites[51].alpha, 0);
  CHECK_EQ(gosteksprites[52].cx, 0.040000f);
  CHECK_EQ(gosteksprites[52].image, 33);
  CHECK_EQ(gosteksprites[52].p1, 23);
  CHECK_EQ(gosteksprites[52].p2, 24);
  CHECK_EQ(gosteksprites[52].id, "Hair_Dreadlock3");
  CHECK_EQ(gosteksprites[52].cy, -0.300000f);
  CHECK_EQ(gosteksprites[52].flex, 0.000000f);
  CHECK_EQ(gosteksprites[52].flip, false);
  CHECK_EQ(gosteksprites[52].team, true);
  CHECK_EQ(gosteksprites[52].color, 4);
  CHECK_EQ(gosteksprites[52].alpha, 0);
  CHECK_EQ(gosteksprites[53].cx, 0.000000f);
  CHECK_EQ(gosteksprites[53].image, 33);
  CHECK_EQ(gosteksprites[53].p1, 23);
  CHECK_EQ(gosteksprites[53].p2, 24);
  CHECK_EQ(gosteksprites[53].id, "Hair_Dreadlock4");
  CHECK_EQ(gosteksprites[53].cy, -0.900000f);
  CHECK_EQ(gosteksprites[53].flex, 0.000000f);
  CHECK_EQ(gosteksprites[53].flip, false);
  CHECK_EQ(gosteksprites[53].team, true);
  CHECK_EQ(gosteksprites[53].color, 4);
  CHECK_EQ(gosteksprites[53].alpha, 0);
  CHECK_EQ(gosteksprites[54].cx, -0.200000f);
  CHECK_EQ(gosteksprites[54].image, 33);
  CHECK_EQ(gosteksprites[54].p1, 23);
  CHECK_EQ(gosteksprites[54].p2, 24);
  CHECK_EQ(gosteksprites[54].id, "Hair_Dreadlock5");
  CHECK_EQ(gosteksprites[54].cy, -1.350000f);
  CHECK_EQ(gosteksprites[54].flex, 0.000000f);
  CHECK_EQ(gosteksprites[54].flip, false);
  CHECK_EQ(gosteksprites[54].team, true);
  CHECK_EQ(gosteksprites[54].color, 4);
  CHECK_EQ(gosteksprites[54].alpha, 0);
  CHECK_EQ(gosteksprites[55].cx, 0.000000f);
  CHECK_EQ(gosteksprites[55].image, 36);
  CHECK_EQ(gosteksprites[55].p1, 9);
  CHECK_EQ(gosteksprites[55].p2, 12);
  CHECK_EQ(gosteksprites[55].id, "Hair_Punk");
  CHECK_EQ(gosteksprites[55].cy, 0.500000f);
  CHECK_EQ(gosteksprites[55].flex, 0.000000f);
  CHECK_EQ(gosteksprites[55].flip, true);
  CHECK_EQ(gosteksprites[55].team, true);
  CHECK_EQ(gosteksprites[55].color, 4);
  CHECK_EQ(gosteksprites[55].alpha, 0);
  CHECK_EQ(gosteksprites[56].cx, 0.000000f);
  CHECK_EQ(gosteksprites[56].image, 40);
  CHECK_EQ(gosteksprites[56].p1, 9);
  CHECK_EQ(gosteksprites[56].p2, 12);
  CHECK_EQ(gosteksprites[56].id, "Hair_Normal");
  CHECK_EQ(gosteksprites[56].cy, 0.500000f);
  CHECK_EQ(gosteksprites[56].flex, 0.000000f);
  CHECK_EQ(gosteksprites[56].flip, true);
  CHECK_EQ(gosteksprites[56].team, true);
  CHECK_EQ(gosteksprites[56].color, 4);
  CHECK_EQ(gosteksprites[56].alpha, 0);
  CHECK_EQ(gosteksprites[57].cx, -0.125000f);
  CHECK_EQ(gosteksprites[57].image, 24);
  CHECK_EQ(gosteksprites[57].p1, 9);
  CHECK_EQ(gosteksprites[57].p2, 12);
  CHECK_EQ(gosteksprites[57].id, "Cigar");
  CHECK_EQ(gosteksprites[57].cy, 0.400000f);
  CHECK_EQ(gosteksprites[57].flex, 0.000000f);
  CHECK_EQ(gosteksprites[57].flip, true);
  CHECK_EQ(gosteksprites[57].team, true);
  CHECK_EQ(gosteksprites[57].color, 5);
  CHECK_EQ(gosteksprites[57].alpha, 0);
  CHECK_EQ(gosteksprites[58].cx, 0.100000f);
  CHECK_EQ(gosteksprites[58].image, 19);
  CHECK_EQ(gosteksprites[58].p1, 10);
  CHECK_EQ(gosteksprites[58].p2, 22);
  CHECK_EQ(gosteksprites[58].id, "Silver_Lchain");
  CHECK_EQ(gosteksprites[58].cy, 0.500000f);
  CHECK_EQ(gosteksprites[58].flex, 0.000000f);
  CHECK_EQ(gosteksprites[58].flip, false);
  CHECK_EQ(gosteksprites[58].team, true);
  CHECK_EQ(gosteksprites[58].color, 0);
  CHECK_EQ(gosteksprites[58].alpha, 0);
  CHECK_EQ(gosteksprites[59].cx, 0.100000f);
  CHECK_EQ(gosteksprites[59].image, 19);
  CHECK_EQ(gosteksprites[59].p1, 11);
  CHECK_EQ(gosteksprites[59].p2, 22);
  CHECK_EQ(gosteksprites[59].id, "Silver_Rchain");
  CHECK_EQ(gosteksprites[59].cy, 0.500000f);
  CHECK_EQ(gosteksprites[59].flex, 0.000000f);
  CHECK_EQ(gosteksprites[59].flip, false);
  CHECK_EQ(gosteksprites[59].team, true);
  CHECK_EQ(gosteksprites[59].color, 0);
  CHECK_EQ(gosteksprites[59].alpha, 0);
  CHECK_EQ(gosteksprites[60].cx, 0.500000f);
  CHECK_EQ(gosteksprites[60].image, 26);
  CHECK_EQ(gosteksprites[60].p1, 22);
  CHECK_EQ(gosteksprites[60].p2, 21);
  CHECK_EQ(gosteksprites[60].id, "Silver_Pendant");
  CHECK_EQ(gosteksprites[60].cy, 0.700000f);
  CHECK_EQ(gosteksprites[60].flex, 0.000000f);
  CHECK_EQ(gosteksprites[60].flip, true);
  CHECK_EQ(gosteksprites[60].team, true);
  CHECK_EQ(gosteksprites[60].color, 0);
  CHECK_EQ(gosteksprites[60].alpha, 0);
  CHECK_EQ(gosteksprites[61].cx, 0.100000f);
  CHECK_EQ(gosteksprites[61].image, 30);
  CHECK_EQ(gosteksprites[61].p1, 10);
  CHECK_EQ(gosteksprites[61].p2, 22);
  CHECK_EQ(gosteksprites[61].id, "Golden_Lchain");
  CHECK_EQ(gosteksprites[61].cy, 0.500000f);
  CHECK_EQ(gosteksprites[61].flex, 0.000000f);
  CHECK_EQ(gosteksprites[61].flip, false);
  CHECK_EQ(gosteksprites[61].team, true);
  CHECK_EQ(gosteksprites[61].color, 0);
  CHECK_EQ(gosteksprites[61].alpha, 0);
  CHECK_EQ(gosteksprites[62].cx, 0.100000f);
  CHECK_EQ(gosteksprites[62].image, 30);
  CHECK_EQ(gosteksprites[62].p1, 11);
  CHECK_EQ(gosteksprites[62].p2, 22);
  CHECK_EQ(gosteksprites[62].id, "Golden_Rchain");
  CHECK_EQ(gosteksprites[62].cy, 0.500000f);
  CHECK_EQ(gosteksprites[62].flex, 0.000000f);
  CHECK_EQ(gosteksprites[62].flip, false);
  CHECK_EQ(gosteksprites[62].team, true);
  CHECK_EQ(gosteksprites[62].color, 0);
  CHECK_EQ(gosteksprites[62].alpha, 0);
  CHECK_EQ(gosteksprites[63].cx, 0.500000f);
  CHECK_EQ(gosteksprites[63].image, 28);
  CHECK_EQ(gosteksprites[63].p1, 22);
  CHECK_EQ(gosteksprites[63].p2, 21);
  CHECK_EQ(gosteksprites[63].id, "Golden_Pendant");
  CHECK_EQ(gosteksprites[63].cy, 0.500000f);
  CHECK_EQ(gosteksprites[63].flex, 0.000000f);
  CHECK_EQ(gosteksprites[63].flip, true);
  CHECK_EQ(gosteksprites[63].team, true);
  CHECK_EQ(gosteksprites[63].color, 0);
  CHECK_EQ(gosteksprites[63].alpha, 0);
  CHECK_EQ(gosteksprites[64].cx, 0.500000f);
  CHECK_EQ(gosteksprites[64].image, 128);
  CHECK_EQ(gosteksprites[64].p1, 5);
  CHECK_EQ(gosteksprites[64].p2, 6);
  CHECK_EQ(gosteksprites[64].id, "Frag_Grenade1");
  CHECK_EQ(gosteksprites[64].cy, 0.100000f);
  CHECK_EQ(gosteksprites[64].flex, 0.000000f);
  CHECK_EQ(gosteksprites[64].flip, false);
  CHECK_EQ(gosteksprites[64].team, false);
  CHECK_EQ(gosteksprites[64].color, 0);
  CHECK_EQ(gosteksprites[64].alpha, 2);
  CHECK_EQ(gosteksprites[65].cx, 0.500000f);
  CHECK_EQ(gosteksprites[65].image, 128);
  CHECK_EQ(gosteksprites[65].p1, 5);
  CHECK_EQ(gosteksprites[65].p2, 6);
  CHECK_EQ(gosteksprites[65].id, "Frag_Grenade2");
  CHECK_EQ(gosteksprites[65].cy, 0.100000f);
  CHECK_EQ(gosteksprites[65].flex, 0.000000f);
  CHECK_EQ(gosteksprites[65].flip, false);
  CHECK_EQ(gosteksprites[65].team, false);
  CHECK_EQ(gosteksprites[65].color, 0);
  CHECK_EQ(gosteksprites[65].alpha, 2);
  CHECK_EQ(gosteksprites[66].cx, 0.500000f);
  CHECK_EQ(gosteksprites[66].image, 128);
  CHECK_EQ(gosteksprites[66].p1, 5);
  CHECK_EQ(gosteksprites[66].p2, 6);
  CHECK_EQ(gosteksprites[66].id, "Frag_Grenade3");
  CHECK_EQ(gosteksprites[66].cy, 0.100000f);
  CHECK_EQ(gosteksprites[66].flex, 0.000000f);
  CHECK_EQ(gosteksprites[66].flip, false);
  CHECK_EQ(gosteksprites[66].team, false);
  CHECK_EQ(gosteksprites[66].color, 0);
  CHECK_EQ(gosteksprites[66].alpha, 2);
  CHECK_EQ(gosteksprites[67].cx, 0.500000f);
  CHECK_EQ(gosteksprites[67].image, 128);
  CHECK_EQ(gosteksprites[67].p1, 5);
  CHECK_EQ(gosteksprites[67].p2, 6);
  CHECK_EQ(gosteksprites[67].id, "Frag_Grenade4");
  CHECK_EQ(gosteksprites[67].cy, 0.100000f);
  CHECK_EQ(gosteksprites[67].flex, 0.000000f);
  CHECK_EQ(gosteksprites[67].flip, false);
  CHECK_EQ(gosteksprites[67].team, false);
  CHECK_EQ(gosteksprites[67].color, 0);
  CHECK_EQ(gosteksprites[67].alpha, 2);
  CHECK_EQ(gosteksprites[68].cx, 0.500000f);
  CHECK_EQ(gosteksprites[68].image, 128);
  CHECK_EQ(gosteksprites[68].p1, 5);
  CHECK_EQ(gosteksprites[68].p2, 6);
  CHECK_EQ(gosteksprites[68].id, "Frag_Grenade5");
  CHECK_EQ(gosteksprites[68].cy, 0.100000f);
  CHECK_EQ(gosteksprites[68].flex, 0.000000f);
  CHECK_EQ(gosteksprites[68].flip, false);
  CHECK_EQ(gosteksprites[68].team, false);
  CHECK_EQ(gosteksprites[68].color, 0);
  CHECK_EQ(gosteksprites[68].alpha, 2);
  CHECK_EQ(gosteksprites[69].cx, 0.500000f);
  CHECK_EQ(gosteksprites[69].image, 127);
  CHECK_EQ(gosteksprites[69].p1, 5);
  CHECK_EQ(gosteksprites[69].p2, 6);
  CHECK_EQ(gosteksprites[69].id, "Cluster_Grenade1");
  CHECK_EQ(gosteksprites[69].cy, 0.300000f);
  CHECK_EQ(gosteksprites[69].flex, 0.000000f);
  CHECK_EQ(gosteksprites[69].flip, false);
  CHECK_EQ(gosteksprites[69].team, false);
  CHECK_EQ(gosteksprites[69].color, 0);
  CHECK_EQ(gosteksprites[69].alpha, 2);
  CHECK_EQ(gosteksprites[70].cx, 0.500000f);
  CHECK_EQ(gosteksprites[70].image, 127);
  CHECK_EQ(gosteksprites[70].p1, 5);
  CHECK_EQ(gosteksprites[70].p2, 6);
  CHECK_EQ(gosteksprites[70].id, "Cluster_Grenade2");
  CHECK_EQ(gosteksprites[70].cy, 0.300000f);
  CHECK_EQ(gosteksprites[70].flex, 0.000000f);
  CHECK_EQ(gosteksprites[70].flip, false);
  CHECK_EQ(gosteksprites[70].team, false);
  CHECK_EQ(gosteksprites[70].color, 0);
  CHECK_EQ(gosteksprites[70].alpha, 2);
  CHECK_EQ(gosteksprites[71].cx, 0.500000f);
  CHECK_EQ(gosteksprites[71].image, 127);
  CHECK_EQ(gosteksprites[71].p1, 5);
  CHECK_EQ(gosteksprites[71].p2, 6);
  CHECK_EQ(gosteksprites[71].id, "Cluster_Grenade3");
  CHECK_EQ(gosteksprites[71].cy, 0.300000f);
  CHECK_EQ(gosteksprites[71].flex, 0.000000f);
  CHECK_EQ(gosteksprites[71].flip, false);
  CHECK_EQ(gosteksprites[71].team, false);
  CHECK_EQ(gosteksprites[71].color, 0);
  CHECK_EQ(gosteksprites[71].alpha, 2);
  CHECK_EQ(gosteksprites[72].cx, 0.500000f);
  CHECK_EQ(gosteksprites[72].image, 127);
  CHECK_EQ(gosteksprites[72].p1, 5);
  CHECK_EQ(gosteksprites[72].p2, 6);
  CHECK_EQ(gosteksprites[72].id, "Cluster_Grenade4");
  CHECK_EQ(gosteksprites[72].cy, 0.300000f);
  CHECK_EQ(gosteksprites[72].flex, 0.000000f);
  CHECK_EQ(gosteksprites[72].flip, false);
  CHECK_EQ(gosteksprites[72].team, false);
  CHECK_EQ(gosteksprites[72].color, 0);
  CHECK_EQ(gosteksprites[72].alpha, 2);
  CHECK_EQ(gosteksprites[73].cx, 0.500000f);
  CHECK_EQ(gosteksprites[73].image, 127);
  CHECK_EQ(gosteksprites[73].p1, 5);
  CHECK_EQ(gosteksprites[73].p2, 6);
  CHECK_EQ(gosteksprites[73].id, "Cluster_Grenade5");
  CHECK_EQ(gosteksprites[73].cy, 0.300000f);
  CHECK_EQ(gosteksprites[73].flex, 0.000000f);
  CHECK_EQ(gosteksprites[73].flip, false);
  CHECK_EQ(gosteksprites[73].team, false);
  CHECK_EQ(gosteksprites[73].color, 0);
  CHECK_EQ(gosteksprites[73].alpha, 2);
  CHECK_EQ(gosteksprites[74].cx, 0.100000f);
  CHECK_EQ(gosteksprites[74].image, 167);
  CHECK_EQ(gosteksprites[74].p1, 16);
  CHECK_EQ(gosteksprites[74].p2, 15);
  CHECK_EQ(gosteksprites[74].id, "Primary_Deagles");
  CHECK_EQ(gosteksprites[74].cy, 0.800000f);
  CHECK_EQ(gosteksprites[74].flex, 0.000000f);
  CHECK_EQ(gosteksprites[74].flip, true);
  CHECK_EQ(gosteksprites[74].team, false);
  CHECK_EQ(gosteksprites[74].color, 0);
  CHECK_EQ(gosteksprites[74].alpha, 0);
  CHECK_EQ(gosteksprites[75].cx, 0.100000f);
  CHECK_EQ(gosteksprites[75].image, 171);
  CHECK_EQ(gosteksprites[75].p1, 16);
  CHECK_EQ(gosteksprites[75].p2, 15);
  CHECK_EQ(gosteksprites[75].id, "Primary_Deagles_Clip");
  CHECK_EQ(gosteksprites[75].cy, 0.800000f);
  CHECK_EQ(gosteksprites[75].flex, 0.000000f);
  CHECK_EQ(gosteksprites[75].flip, true);
  CHECK_EQ(gosteksprites[75].team, false);
  CHECK_EQ(gosteksprites[75].color, 0);
  CHECK_EQ(gosteksprites[75].alpha, 0);
  CHECK_EQ(gosteksprites[76].cx, -0.500000f);
  CHECK_EQ(gosteksprites[76].image, 175);
  CHECK_EQ(gosteksprites[76].p1, 16);
  CHECK_EQ(gosteksprites[76].p2, 15);
  CHECK_EQ(gosteksprites[76].id, "Primary_Deagles_Fire");
  CHECK_EQ(gosteksprites[76].cy, 1.000000f);
  CHECK_EQ(gosteksprites[76].flex, 0.000000f);
  CHECK_EQ(gosteksprites[76].flip, false);
  CHECK_EQ(gosteksprites[76].team, false);
  CHECK_EQ(gosteksprites[76].color, 0);
  CHECK_EQ(gosteksprites[76].alpha, 0);
  CHECK_EQ(gosteksprites[77].cx, 0.150000f);
  CHECK_EQ(gosteksprites[77].image, 148);
  CHECK_EQ(gosteksprites[77].p1, 16);
  CHECK_EQ(gosteksprites[77].p2, 15);
  CHECK_EQ(gosteksprites[77].id, "Primary_Mp5");
  CHECK_EQ(gosteksprites[77].cy, 0.600000f);
  CHECK_EQ(gosteksprites[77].flex, 0.000000f);
  CHECK_EQ(gosteksprites[77].flip, true);
  CHECK_EQ(gosteksprites[77].team, false);
  CHECK_EQ(gosteksprites[77].color, 0);
  CHECK_EQ(gosteksprites[77].alpha, 0);
  CHECK_EQ(gosteksprites[78].cx, 0.150000f);
  CHECK_EQ(gosteksprites[78].image, 150);
  CHECK_EQ(gosteksprites[78].p1, 16);
  CHECK_EQ(gosteksprites[78].p2, 15);
  CHECK_EQ(gosteksprites[78].id, "Primary_Mp5_Clip");
  CHECK_EQ(gosteksprites[78].cy, 0.600000f);
  CHECK_EQ(gosteksprites[78].flex, 0.000000f);
  CHECK_EQ(gosteksprites[78].flip, true);
  CHECK_EQ(gosteksprites[78].team, false);
  CHECK_EQ(gosteksprites[78].color, 0);
  CHECK_EQ(gosteksprites[78].alpha, 0);
  CHECK_EQ(gosteksprites[79].cx, -0.650000f);
  CHECK_EQ(gosteksprites[79].image, 154);
  CHECK_EQ(gosteksprites[79].p1, 16);
  CHECK_EQ(gosteksprites[79].p2, 15);
  CHECK_EQ(gosteksprites[79].id, "Primary_Mp5_Fire");
  CHECK_EQ(gosteksprites[79].cy, 0.850000f);
  CHECK_EQ(gosteksprites[79].flex, 0.000000f);
  CHECK_EQ(gosteksprites[79].flip, false);
  CHECK_EQ(gosteksprites[79].team, false);
  CHECK_EQ(gosteksprites[79].color, 0);
  CHECK_EQ(gosteksprites[79].alpha, 0);
  CHECK_EQ(gosteksprites[80].cx, 0.150000f);
  CHECK_EQ(gosteksprites[80].image, 129);
  CHECK_EQ(gosteksprites[80].p1, 16);
  CHECK_EQ(gosteksprites[80].p2, 15);
  CHECK_EQ(gosteksprites[80].id, "Primary_Ak74");
  CHECK_EQ(gosteksprites[80].cy, 0.500000f);
  CHECK_EQ(gosteksprites[80].flex, 0.000000f);
  CHECK_EQ(gosteksprites[80].flip, true);
  CHECK_EQ(gosteksprites[80].team, false);
  CHECK_EQ(gosteksprites[80].color, 0);
  CHECK_EQ(gosteksprites[80].alpha, 0);
  CHECK_EQ(gosteksprites[81].cx, 0.150000f);
  CHECK_EQ(gosteksprites[81].image, 131);
  CHECK_EQ(gosteksprites[81].p1, 16);
  CHECK_EQ(gosteksprites[81].p2, 15);
  CHECK_EQ(gosteksprites[81].id, "Primary_Ak74_Clip");
  CHECK_EQ(gosteksprites[81].cy, 0.500000f);
  CHECK_EQ(gosteksprites[81].flex, 0.000000f);
  CHECK_EQ(gosteksprites[81].flip, true);
  CHECK_EQ(gosteksprites[81].team, false);
  CHECK_EQ(gosteksprites[81].color, 0);
  CHECK_EQ(gosteksprites[81].alpha, 0);
  CHECK_EQ(gosteksprites[82].cx, -0.370000f);
  CHECK_EQ(gosteksprites[82].image, 135);
  CHECK_EQ(gosteksprites[82].p1, 16);
  CHECK_EQ(gosteksprites[82].p2, 15);
  CHECK_EQ(gosteksprites[82].id, "Primary_Ak74_Fire");
  CHECK_EQ(gosteksprites[82].cy, 0.800000f);
  CHECK_EQ(gosteksprites[82].flex, 0.000000f);
  CHECK_EQ(gosteksprites[82].flip, false);
  CHECK_EQ(gosteksprites[82].team, false);
  CHECK_EQ(gosteksprites[82].color, 0);
  CHECK_EQ(gosteksprites[82].alpha, 0);
  CHECK_EQ(gosteksprites[83].cx, 0.200000f);
  CHECK_EQ(gosteksprites[83].image, 176);
  CHECK_EQ(gosteksprites[83].p1, 16);
  CHECK_EQ(gosteksprites[83].p2, 15);
  CHECK_EQ(gosteksprites[83].id, "Primary_Steyr");
  CHECK_EQ(gosteksprites[83].cy, 0.600000f);
  CHECK_EQ(gosteksprites[83].flex, 0.000000f);
  CHECK_EQ(gosteksprites[83].flip, true);
  CHECK_EQ(gosteksprites[83].team, false);
  CHECK_EQ(gosteksprites[83].color, 0);
  CHECK_EQ(gosteksprites[83].alpha, 0);
  CHECK_EQ(gosteksprites[84].cx, 0.200000f);
  CHECK_EQ(gosteksprites[84].image, 178);
  CHECK_EQ(gosteksprites[84].p1, 16);
  CHECK_EQ(gosteksprites[84].p2, 15);
  CHECK_EQ(gosteksprites[84].id, "Primary_Steyr_Clip");
  CHECK_EQ(gosteksprites[84].cy, 0.600000f);
  CHECK_EQ(gosteksprites[84].flex, 0.000000f);
  CHECK_EQ(gosteksprites[84].flip, true);
  CHECK_EQ(gosteksprites[84].team, false);
  CHECK_EQ(gosteksprites[84].color, 0);
  CHECK_EQ(gosteksprites[84].alpha, 0);
  CHECK_EQ(gosteksprites[85].cx, -0.240000f);
  CHECK_EQ(gosteksprites[85].image, 182);
  CHECK_EQ(gosteksprites[85].p1, 16);
  CHECK_EQ(gosteksprites[85].p2, 15);
  CHECK_EQ(gosteksprites[85].id, "Primary_Steyr_Fire");
  CHECK_EQ(gosteksprites[85].cy, 0.750000f);
  CHECK_EQ(gosteksprites[85].flex, 0.000000f);
  CHECK_EQ(gosteksprites[85].flip, false);
  CHECK_EQ(gosteksprites[85].team, false);
  CHECK_EQ(gosteksprites[85].color, 0);
  CHECK_EQ(gosteksprites[85].alpha, 0);
  CHECK_EQ(gosteksprites[86].cx, 0.100000f);
  CHECK_EQ(gosteksprites[86].image, 155);
  CHECK_EQ(gosteksprites[86].p1, 16);
  CHECK_EQ(gosteksprites[86].p2, 15);
  CHECK_EQ(gosteksprites[86].id, "Primary_Spas");
  CHECK_EQ(gosteksprites[86].cy, 0.600000f);
  CHECK_EQ(gosteksprites[86].flex, 0.000000f);
  CHECK_EQ(gosteksprites[86].flip, true);
  CHECK_EQ(gosteksprites[86].team, false);
  CHECK_EQ(gosteksprites[86].color, 0);
  CHECK_EQ(gosteksprites[86].alpha, 0);
  CHECK_EQ(gosteksprites[87].cx, 0.000000f);
  CHECK_EQ(gosteksprites[87].image, 0);
  CHECK_EQ(gosteksprites[87].p1, 16);
  CHECK_EQ(gosteksprites[87].p2, 15);
  CHECK_EQ(gosteksprites[87].id, "Primary_Spas_Clip");
  CHECK_EQ(gosteksprites[87].cy, 0.000000f);
  CHECK_EQ(gosteksprites[87].flex, 0.000000f);
  CHECK_EQ(gosteksprites[87].flip, true);
  CHECK_EQ(gosteksprites[87].team, false);
  CHECK_EQ(gosteksprites[87].color, 0);
  CHECK_EQ(gosteksprites[87].alpha, 0);
  CHECK_EQ(gosteksprites[88].cx, -0.200000f);
  CHECK_EQ(gosteksprites[88].image, 159);
  CHECK_EQ(gosteksprites[88].p1, 16);
  CHECK_EQ(gosteksprites[88].p2, 15);
  CHECK_EQ(gosteksprites[88].id, "Primary_Spas_Fire");
  CHECK_EQ(gosteksprites[88].cy, 0.900000f);
  CHECK_EQ(gosteksprites[88].flex, 0.000000f);
  CHECK_EQ(gosteksprites[88].flip, false);
  CHECK_EQ(gosteksprites[88].team, false);
  CHECK_EQ(gosteksprites[88].color, 0);
  CHECK_EQ(gosteksprites[88].alpha, 0);
  CHECK_EQ(gosteksprites[89].cx, 0.100000f);
  CHECK_EQ(gosteksprites[89].image, 143);
  CHECK_EQ(gosteksprites[89].p1, 16);
  CHECK_EQ(gosteksprites[89].p2, 15);
  CHECK_EQ(gosteksprites[89].id, "Primary_Ruger");
  CHECK_EQ(gosteksprites[89].cy, 0.700000f);
  CHECK_EQ(gosteksprites[89].flex, 0.000000f);
  CHECK_EQ(gosteksprites[89].flip, true);
  CHECK_EQ(gosteksprites[89].team, false);
  CHECK_EQ(gosteksprites[89].color, 0);
  CHECK_EQ(gosteksprites[89].alpha, 0);
  CHECK_EQ(gosteksprites[90].cx, 0.000000f);
  CHECK_EQ(gosteksprites[90].image, 0);
  CHECK_EQ(gosteksprites[90].p1, 16);
  CHECK_EQ(gosteksprites[90].p2, 15);
  CHECK_EQ(gosteksprites[90].id, "Primary_Ruger_Clip");
  CHECK_EQ(gosteksprites[90].cy, 0.000000f);
  CHECK_EQ(gosteksprites[90].flex, 0.000000f);
  CHECK_EQ(gosteksprites[90].flip, true);
  CHECK_EQ(gosteksprites[90].team, false);
  CHECK_EQ(gosteksprites[90].color, 0);
  CHECK_EQ(gosteksprites[90].alpha, 0);
  CHECK_EQ(gosteksprites[91].cx, -0.350000f);
  CHECK_EQ(gosteksprites[91].image, 147);
  CHECK_EQ(gosteksprites[91].p1, 16);
  CHECK_EQ(gosteksprites[91].p2, 15);
  CHECK_EQ(gosteksprites[91].id, "Primary_Ruger_Fire");
  CHECK_EQ(gosteksprites[91].cy, 0.850000f);
  CHECK_EQ(gosteksprites[91].flex, 0.000000f);
  CHECK_EQ(gosteksprites[91].flip, false);
  CHECK_EQ(gosteksprites[91].team, false);
  CHECK_EQ(gosteksprites[91].color, 0);
  CHECK_EQ(gosteksprites[91].alpha, 0);
  CHECK_EQ(gosteksprites[92].cx, 0.100000f);
  CHECK_EQ(gosteksprites[92].image, 160);
  CHECK_EQ(gosteksprites[92].p1, 16);
  CHECK_EQ(gosteksprites[92].p2, 15);
  CHECK_EQ(gosteksprites[92].id, "Primary_M79");
  CHECK_EQ(gosteksprites[92].cy, 0.700000f);
  CHECK_EQ(gosteksprites[92].flex, 0.000000f);
  CHECK_EQ(gosteksprites[92].flip, true);
  CHECK_EQ(gosteksprites[92].team, false);
  CHECK_EQ(gosteksprites[92].color, 0);
  CHECK_EQ(gosteksprites[92].alpha, 0);
  CHECK_EQ(gosteksprites[93].cx, 0.100000f);
  CHECK_EQ(gosteksprites[93].image, 162);
  CHECK_EQ(gosteksprites[93].p1, 16);
  CHECK_EQ(gosteksprites[93].p2, 15);
  CHECK_EQ(gosteksprites[93].id, "Primary_M79_Clip");
  CHECK_EQ(gosteksprites[93].cy, 0.700000f);
  CHECK_EQ(gosteksprites[93].flex, 0.000000f);
  CHECK_EQ(gosteksprites[93].flip, true);
  CHECK_EQ(gosteksprites[93].team, false);
  CHECK_EQ(gosteksprites[93].color, 0);
  CHECK_EQ(gosteksprites[93].alpha, 0);
  CHECK_EQ(gosteksprites[94].cx, -0.400000f);
  CHECK_EQ(gosteksprites[94].image, 166);
  CHECK_EQ(gosteksprites[94].p1, 16);
  CHECK_EQ(gosteksprites[94].p2, 15);
  CHECK_EQ(gosteksprites[94].id, "Primary_M79_Fire");
  CHECK_EQ(gosteksprites[94].cy, 0.800000f);
  CHECK_EQ(gosteksprites[94].flex, 0.000000f);
  CHECK_EQ(gosteksprites[94].flip, false);
  CHECK_EQ(gosteksprites[94].team, false);
  CHECK_EQ(gosteksprites[94].color, 0);
  CHECK_EQ(gosteksprites[94].alpha, 0);
  CHECK_EQ(gosteksprites[95].cx, 0.150000f);
  CHECK_EQ(gosteksprites[95].image, 183);
  CHECK_EQ(gosteksprites[95].p1, 16);
  CHECK_EQ(gosteksprites[95].p2, 15);
  CHECK_EQ(gosteksprites[95].id, "Primary_Barrett");
  CHECK_EQ(gosteksprites[95].cy, 0.700000f);
  CHECK_EQ(gosteksprites[95].flex, 0.000000f);
  CHECK_EQ(gosteksprites[95].flip, true);
  CHECK_EQ(gosteksprites[95].team, false);
  CHECK_EQ(gosteksprites[95].color, 0);
  CHECK_EQ(gosteksprites[95].alpha, 0);
  CHECK_EQ(gosteksprites[96].cx, 0.150000f);
  CHECK_EQ(gosteksprites[96].image, 185);
  CHECK_EQ(gosteksprites[96].p1, 16);
  CHECK_EQ(gosteksprites[96].p2, 15);
  CHECK_EQ(gosteksprites[96].id, "Primary_Barrett_Clip");
  CHECK_EQ(gosteksprites[96].cy, 0.700000f);
  CHECK_EQ(gosteksprites[96].flex, 0.000000f);
  CHECK_EQ(gosteksprites[96].flip, true);
  CHECK_EQ(gosteksprites[96].team, false);
  CHECK_EQ(gosteksprites[96].color, 0);
  CHECK_EQ(gosteksprites[96].alpha, 0);
  CHECK_EQ(gosteksprites[97].cx, -0.150000f);
  CHECK_EQ(gosteksprites[97].image, 189);
  CHECK_EQ(gosteksprites[97].p1, 16);
  CHECK_EQ(gosteksprites[97].p2, 15);
  CHECK_EQ(gosteksprites[97].id, "Primary_Barrett_Fire");
  CHECK_EQ(gosteksprites[97].cy, 0.800000f);
  CHECK_EQ(gosteksprites[97].flex, 0.000000f);
  CHECK_EQ(gosteksprites[97].flip, false);
  CHECK_EQ(gosteksprites[97].team, false);
  CHECK_EQ(gosteksprites[97].color, 0);
  CHECK_EQ(gosteksprites[97].alpha, 0);
  CHECK_EQ(gosteksprites[98].cx, 0.150000f);
  CHECK_EQ(gosteksprites[98].image, 136);
  CHECK_EQ(gosteksprites[98].p1, 16);
  CHECK_EQ(gosteksprites[98].p2, 15);
  CHECK_EQ(gosteksprites[98].id, "Primary_Minimi");
  CHECK_EQ(gosteksprites[98].cy, 0.600000f);
  CHECK_EQ(gosteksprites[98].flex, 0.000000f);
  CHECK_EQ(gosteksprites[98].flip, true);
  CHECK_EQ(gosteksprites[98].team, false);
  CHECK_EQ(gosteksprites[98].color, 0);
  CHECK_EQ(gosteksprites[98].alpha, 0);
  CHECK_EQ(gosteksprites[99].cx, 0.150000f);
  CHECK_EQ(gosteksprites[99].image, 138);
  CHECK_EQ(gosteksprites[99].p1, 16);
  CHECK_EQ(gosteksprites[99].p2, 15);
  CHECK_EQ(gosteksprites[99].id, "Primary_Minimi_Clip");
  CHECK_EQ(gosteksprites[99].cy, 0.600000f);
  CHECK_EQ(gosteksprites[99].flex, 0.000000f);
  CHECK_EQ(gosteksprites[99].flip, true);
  CHECK_EQ(gosteksprites[99].team, false);
  CHECK_EQ(gosteksprites[99].color, 0);
  CHECK_EQ(gosteksprites[99].alpha, 0);
  CHECK_EQ(gosteksprites[100].cx, -0.200000f);
  CHECK_EQ(gosteksprites[100].image, 142);
  CHECK_EQ(gosteksprites[100].p1, 16);
  CHECK_EQ(gosteksprites[100].p2, 15);
  CHECK_EQ(gosteksprites[100].id, "Primary_Minimi_Fire");
  CHECK_EQ(gosteksprites[100].cy, 0.900000f);
  CHECK_EQ(gosteksprites[100].flex, 0.000000f);
  CHECK_EQ(gosteksprites[100].flip, false);
  CHECK_EQ(gosteksprites[100].team, false);
  CHECK_EQ(gosteksprites[100].color, 0);
  CHECK_EQ(gosteksprites[100].alpha, 0);
  CHECK_EQ(gosteksprites[101].cx, 0.500000f);
  CHECK_EQ(gosteksprites[101].image, 192);
  CHECK_EQ(gosteksprites[101].p1, 8);
  CHECK_EQ(gosteksprites[101].p2, 7);
  CHECK_EQ(gosteksprites[101].id, "Primary_Minigun_Clip");
  CHECK_EQ(gosteksprites[101].cy, 0.100000f);
  CHECK_EQ(gosteksprites[101].flex, 0.000000f);
  CHECK_EQ(gosteksprites[101].flip, false);
  CHECK_EQ(gosteksprites[101].team, false);
  CHECK_EQ(gosteksprites[101].color, 0);
  CHECK_EQ(gosteksprites[101].alpha, 0);
  CHECK_EQ(gosteksprites[102].cx, 0.050000f);
  CHECK_EQ(gosteksprites[102].image, 190);
  CHECK_EQ(gosteksprites[102].p1, 16);
  CHECK_EQ(gosteksprites[102].p2, 15);
  CHECK_EQ(gosteksprites[102].id, "Primary_Minigun");
  CHECK_EQ(gosteksprites[102].cy, 0.500000f);
  CHECK_EQ(gosteksprites[102].flex, 0.000000f);
  CHECK_EQ(gosteksprites[102].flip, true);
  CHECK_EQ(gosteksprites[102].team, false);
  CHECK_EQ(gosteksprites[102].color, 0);
  CHECK_EQ(gosteksprites[102].alpha, 0);
  CHECK_EQ(gosteksprites[103].cx, -0.200000f);
  CHECK_EQ(gosteksprites[103].image, 195);
  CHECK_EQ(gosteksprites[103].p1, 16);
  CHECK_EQ(gosteksprites[103].p2, 15);
  CHECK_EQ(gosteksprites[103].id, "Primary_Minigun_Fire");
  CHECK_EQ(gosteksprites[103].cy, 0.450000f);
  CHECK_EQ(gosteksprites[103].flex, 0.000000f);
  CHECK_EQ(gosteksprites[103].flip, false);
  CHECK_EQ(gosteksprites[103].team, false);
  CHECK_EQ(gosteksprites[103].color, 0);
  CHECK_EQ(gosteksprites[103].alpha, 0);
  CHECK_EQ(gosteksprites[104].cx, 0.200000f);
  CHECK_EQ(gosteksprites[104].image, 196);
  CHECK_EQ(gosteksprites[104].p1, 16);
  CHECK_EQ(gosteksprites[104].p2, 15);
  CHECK_EQ(gosteksprites[104].id, "Primary_Socom");
  CHECK_EQ(gosteksprites[104].cy, 0.550000f);
  CHECK_EQ(gosteksprites[104].flex, 0.000000f);
  CHECK_EQ(gosteksprites[104].flip, true);
  CHECK_EQ(gosteksprites[104].team, false);
  CHECK_EQ(gosteksprites[104].color, 0);
  CHECK_EQ(gosteksprites[104].alpha, 0);
  CHECK_EQ(gosteksprites[105].cx, 0.200000f);
  CHECK_EQ(gosteksprites[105].image, 200);
  CHECK_EQ(gosteksprites[105].p1, 16);
  CHECK_EQ(gosteksprites[105].p2, 15);
  CHECK_EQ(gosteksprites[105].id, "Primary_Socom_Clip");
  CHECK_EQ(gosteksprites[105].cy, 0.550000f);
  CHECK_EQ(gosteksprites[105].flex, 0.000000f);
  CHECK_EQ(gosteksprites[105].flip, true);
  CHECK_EQ(gosteksprites[105].team, false);
  CHECK_EQ(gosteksprites[105].color, 0);
  CHECK_EQ(gosteksprites[105].alpha, 0);
  CHECK_EQ(gosteksprites[106].cx, -0.240000f);
  CHECK_EQ(gosteksprites[106].image, 204);
  CHECK_EQ(gosteksprites[106].p1, 16);
  CHECK_EQ(gosteksprites[106].p2, 15);
  CHECK_EQ(gosteksprites[106].id, "Primary_Socom_Fire");
  CHECK_EQ(gosteksprites[106].cy, 0.850000f);
  CHECK_EQ(gosteksprites[106].flex, 0.000000f);
  CHECK_EQ(gosteksprites[106].flip, false);
  CHECK_EQ(gosteksprites[106].team, false);
  CHECK_EQ(gosteksprites[106].color, 0);
  CHECK_EQ(gosteksprites[106].alpha, 0);
  CHECK_EQ(gosteksprites[107].cx, -0.100000f);
  CHECK_EQ(gosteksprites[107].image, 216);
  CHECK_EQ(gosteksprites[107].p1, 16);
  CHECK_EQ(gosteksprites[107].p2, 20);
  CHECK_EQ(gosteksprites[107].id, "Primary_Knife");
  CHECK_EQ(gosteksprites[107].cy, 0.600000f);
  CHECK_EQ(gosteksprites[107].flex, 0.000000f);
  CHECK_EQ(gosteksprites[107].flip, true);
  CHECK_EQ(gosteksprites[107].team, false);
  CHECK_EQ(gosteksprites[107].color, 0);
  CHECK_EQ(gosteksprites[107].alpha, 0);
  CHECK_EQ(gosteksprites[108].cx, 0.000000f);
  CHECK_EQ(gosteksprites[108].image, 0);
  CHECK_EQ(gosteksprites[108].p1, 16);
  CHECK_EQ(gosteksprites[108].p2, 15);
  CHECK_EQ(gosteksprites[108].id, "Primary_Knife_Clip");
  CHECK_EQ(gosteksprites[108].cy, 0.000000f);
  CHECK_EQ(gosteksprites[108].flex, 0.000000f);
  CHECK_EQ(gosteksprites[108].flip, true);
  CHECK_EQ(gosteksprites[108].team, false);
  CHECK_EQ(gosteksprites[108].color, 0);
  CHECK_EQ(gosteksprites[108].alpha, 0);
  CHECK_EQ(gosteksprites[109].cx, 0.000000f);
  CHECK_EQ(gosteksprites[109].image, 0);
  CHECK_EQ(gosteksprites[109].p1, 16);
  CHECK_EQ(gosteksprites[109].p2, 15);
  CHECK_EQ(gosteksprites[109].id, "Primary_Knife_Fire");
  CHECK_EQ(gosteksprites[109].cy, 0.000000f);
  CHECK_EQ(gosteksprites[109].flex, 0.000000f);
  CHECK_EQ(gosteksprites[109].flip, true);
  CHECK_EQ(gosteksprites[109].team, false);
  CHECK_EQ(gosteksprites[109].color, 0);
  CHECK_EQ(gosteksprites[109].alpha, 0);
  CHECK_EQ(gosteksprites[110].cx, 0.100000f);
  CHECK_EQ(gosteksprites[110].image, 218);
  CHECK_EQ(gosteksprites[110].p1, 16);
  CHECK_EQ(gosteksprites[110].p2, 15);
  CHECK_EQ(gosteksprites[110].id, "Primary_Chainsaw");
  CHECK_EQ(gosteksprites[110].cy, 0.500000f);
  CHECK_EQ(gosteksprites[110].flex, 0.000000f);
  CHECK_EQ(gosteksprites[110].flip, true);
  CHECK_EQ(gosteksprites[110].team, false);
  CHECK_EQ(gosteksprites[110].color, 0);
  CHECK_EQ(gosteksprites[110].alpha, 0);
  CHECK_EQ(gosteksprites[111].cx, 0.000000f);
  CHECK_EQ(gosteksprites[111].image, 0);
  CHECK_EQ(gosteksprites[111].p1, 16);
  CHECK_EQ(gosteksprites[111].p2, 15);
  CHECK_EQ(gosteksprites[111].id, "Primary_Chainsaw_Clip");
  CHECK_EQ(gosteksprites[111].cy, 0.000000f);
  CHECK_EQ(gosteksprites[111].flex, 0.000000f);
  CHECK_EQ(gosteksprites[111].flip, true);
  CHECK_EQ(gosteksprites[111].team, false);
  CHECK_EQ(gosteksprites[111].color, 0);
  CHECK_EQ(gosteksprites[111].alpha, 0);
  CHECK_EQ(gosteksprites[112].cx, 0.000000f);
  CHECK_EQ(gosteksprites[112].image, 220);
  CHECK_EQ(gosteksprites[112].p1, 16);
  CHECK_EQ(gosteksprites[112].p2, 15);
  CHECK_EQ(gosteksprites[112].id, "Primary_Chainsaw_Fire");
  CHECK_EQ(gosteksprites[112].cy, 0.000000f);
  CHECK_EQ(gosteksprites[112].flex, 0.000000f);
  CHECK_EQ(gosteksprites[112].flip, false);
  CHECK_EQ(gosteksprites[112].team, false);
  CHECK_EQ(gosteksprites[112].color, 0);
  CHECK_EQ(gosteksprites[112].alpha, 0);
  CHECK_EQ(gosteksprites[113].cx, 0.100000f);
  CHECK_EQ(gosteksprites[113].image, 221);
  CHECK_EQ(gosteksprites[113].p1, 16);
  CHECK_EQ(gosteksprites[113].p2, 15);
  CHECK_EQ(gosteksprites[113].id, "Primary_Law");
  CHECK_EQ(gosteksprites[113].cy, 0.600000f);
  CHECK_EQ(gosteksprites[113].flex, 0.000000f);
  CHECK_EQ(gosteksprites[113].flip, true);
  CHECK_EQ(gosteksprites[113].team, false);
  CHECK_EQ(gosteksprites[113].color, 0);
  CHECK_EQ(gosteksprites[113].alpha, 0);
  CHECK_EQ(gosteksprites[114].cx, 0.000000f);
  CHECK_EQ(gosteksprites[114].image, 0);
  CHECK_EQ(gosteksprites[114].p1, 16);
  CHECK_EQ(gosteksprites[114].p2, 15);
  CHECK_EQ(gosteksprites[114].id, "Primary_Law_Clip");
  CHECK_EQ(gosteksprites[114].cy, 0.000000f);
  CHECK_EQ(gosteksprites[114].flex, 0.000000f);
  CHECK_EQ(gosteksprites[114].flip, true);
  CHECK_EQ(gosteksprites[114].team, false);
  CHECK_EQ(gosteksprites[114].color, 0);
  CHECK_EQ(gosteksprites[114].alpha, 0);
  CHECK_EQ(gosteksprites[115].cx, -0.100000f);
  CHECK_EQ(gosteksprites[115].image, 223);
  CHECK_EQ(gosteksprites[115].p1, 16);
  CHECK_EQ(gosteksprites[115].p2, 15);
  CHECK_EQ(gosteksprites[115].id, "Primary_Law_Fire");
  CHECK_EQ(gosteksprites[115].cy, 0.550000f);
  CHECK_EQ(gosteksprites[115].flex, 0.000000f);
  CHECK_EQ(gosteksprites[115].flip, false);
  CHECK_EQ(gosteksprites[115].team, false);
  CHECK_EQ(gosteksprites[115].color, 0);
  CHECK_EQ(gosteksprites[115].alpha, 0);
  CHECK_EQ(gosteksprites[116].cx, -0.400000f);
  CHECK_EQ(gosteksprites[116].image, 205);
  CHECK_EQ(gosteksprites[116].p1, 16);
  CHECK_EQ(gosteksprites[116].p2, 15);
  CHECK_EQ(gosteksprites[116].id, "Primary_Bow");
  CHECK_EQ(gosteksprites[116].cy, 0.550000f);
  CHECK_EQ(gosteksprites[116].flex, 0.000000f);
  CHECK_EQ(gosteksprites[116].flip, true);
  CHECK_EQ(gosteksprites[116].team, false);
  CHECK_EQ(gosteksprites[116].color, 0);
  CHECK_EQ(gosteksprites[116].alpha, 0);
  CHECK_EQ(gosteksprites[117].cx, 0.000000f);
  CHECK_EQ(gosteksprites[117].image, 208);
  CHECK_EQ(gosteksprites[117].p1, 16);
  CHECK_EQ(gosteksprites[117].p2, 15);
  CHECK_EQ(gosteksprites[117].id, "Primary_Bow_Arrow");
  CHECK_EQ(gosteksprites[117].cy, 0.550000f);
  CHECK_EQ(gosteksprites[117].flex, 0.000000f);
  CHECK_EQ(gosteksprites[117].flip, false);
  CHECK_EQ(gosteksprites[117].team, false);
  CHECK_EQ(gosteksprites[117].color, 0);
  CHECK_EQ(gosteksprites[117].alpha, 0);
  CHECK_EQ(gosteksprites[118].cx, -0.400000f);
  CHECK_EQ(gosteksprites[118].image, 207);
  CHECK_EQ(gosteksprites[118].p1, 16);
  CHECK_EQ(gosteksprites[118].p2, 15);
  CHECK_EQ(gosteksprites[118].id, "Primary_Bow_String");
  CHECK_EQ(gosteksprites[118].cy, 0.550000f);
  CHECK_EQ(gosteksprites[118].flex, 0.000000f);
  CHECK_EQ(gosteksprites[118].flip, false);
  CHECK_EQ(gosteksprites[118].team, false);
  CHECK_EQ(gosteksprites[118].color, 0);
  CHECK_EQ(gosteksprites[118].alpha, 0);
  CHECK_EQ(gosteksprites[119].cx, -0.400000f);
  CHECK_EQ(gosteksprites[119].image, 205);
  CHECK_EQ(gosteksprites[119].p1, 16);
  CHECK_EQ(gosteksprites[119].p2, 15);
  CHECK_EQ(gosteksprites[119].id, "Primary_Bow_Reload");
  CHECK_EQ(gosteksprites[119].cy, 0.550000f);
  CHECK_EQ(gosteksprites[119].flex, 0.000000f);
  CHECK_EQ(gosteksprites[119].flip, true);
  CHECK_EQ(gosteksprites[119].team, false);
  CHECK_EQ(gosteksprites[119].color, 0);
  CHECK_EQ(gosteksprites[119].alpha, 0);
  CHECK_EQ(gosteksprites[120].cx, 0.000000f);
  CHECK_EQ(gosteksprites[120].image, 211);
  CHECK_EQ(gosteksprites[120].p1, 16);
  CHECK_EQ(gosteksprites[120].p2, 20);
  CHECK_EQ(gosteksprites[120].id, "Primary_Bow_Arrow_Reload");
  CHECK_EQ(gosteksprites[120].cy, 0.550000f);
  CHECK_EQ(gosteksprites[120].flex, 0.000000f);
  CHECK_EQ(gosteksprites[120].flip, false);
  CHECK_EQ(gosteksprites[120].team, false);
  CHECK_EQ(gosteksprites[120].color, 0);
  CHECK_EQ(gosteksprites[120].alpha, 0);
  CHECK_EQ(gosteksprites[121].cx, -0.400000f);
  CHECK_EQ(gosteksprites[121].image, 207);
  CHECK_EQ(gosteksprites[121].p1, 16);
  CHECK_EQ(gosteksprites[121].p2, 15);
  CHECK_EQ(gosteksprites[121].id, "Primary_Bow_String_Reload");
  CHECK_EQ(gosteksprites[121].cy, 0.550000f);
  CHECK_EQ(gosteksprites[121].flex, 0.000000f);
  CHECK_EQ(gosteksprites[121].flip, false);
  CHECK_EQ(gosteksprites[121].team, false);
  CHECK_EQ(gosteksprites[121].color, 0);
  CHECK_EQ(gosteksprites[121].alpha, 0);
  CHECK_EQ(gosteksprites[122].cx, 0.000000f);
  CHECK_EQ(gosteksprites[122].image, 212);
  CHECK_EQ(gosteksprites[122].p1, 16);
  CHECK_EQ(gosteksprites[122].p2, 15);
  CHECK_EQ(gosteksprites[122].id, "Primary_Bow_Fire");
  CHECK_EQ(gosteksprites[122].cy, 0.000000f);
  CHECK_EQ(gosteksprites[122].flex, 0.000000f);
  CHECK_EQ(gosteksprites[122].flip, false);
  CHECK_EQ(gosteksprites[122].team, false);
  CHECK_EQ(gosteksprites[122].color, 0);
  CHECK_EQ(gosteksprites[122].alpha, 0);
  CHECK_EQ(gosteksprites[123].cx, 0.200000f);
  CHECK_EQ(gosteksprites[123].image, 213);
  CHECK_EQ(gosteksprites[123].p1, 16);
  CHECK_EQ(gosteksprites[123].p2, 15);
  CHECK_EQ(gosteksprites[123].id, "Primary_Flamer");
  CHECK_EQ(gosteksprites[123].cy, 0.700000f);
  CHECK_EQ(gosteksprites[123].flex, 0.000000f);
  CHECK_EQ(gosteksprites[123].flip, true);
  CHECK_EQ(gosteksprites[123].team, false);
  CHECK_EQ(gosteksprites[123].color, 0);
  CHECK_EQ(gosteksprites[123].alpha, 0);
  CHECK_EQ(gosteksprites[124].cx, 0.000000f);
  CHECK_EQ(gosteksprites[124].image, 0);
  CHECK_EQ(gosteksprites[124].p1, 16);
  CHECK_EQ(gosteksprites[124].p2, 15);
  CHECK_EQ(gosteksprites[124].id, "Primary_Flamer_Clip");
  CHECK_EQ(gosteksprites[124].cy, 0.000000f);
  CHECK_EQ(gosteksprites[124].flex, 0.000000f);
  CHECK_EQ(gosteksprites[124].flip, true);
  CHECK_EQ(gosteksprites[124].team, false);
  CHECK_EQ(gosteksprites[124].color, 0);
  CHECK_EQ(gosteksprites[124].alpha, 0);
  CHECK_EQ(gosteksprites[125].cx, 0.000000f);
  CHECK_EQ(gosteksprites[125].image, 215);
  CHECK_EQ(gosteksprites[125].p1, 16);
  CHECK_EQ(gosteksprites[125].p2, 15);
  CHECK_EQ(gosteksprites[125].id, "Primary_Flamer_Fire");
  CHECK_EQ(gosteksprites[125].cy, 0.000000f);
  CHECK_EQ(gosteksprites[125].flex, 0.000000f);
  CHECK_EQ(gosteksprites[125].flip, false);
  CHECK_EQ(gosteksprites[125].team, false);
  CHECK_EQ(gosteksprites[125].color, 0);
  CHECK_EQ(gosteksprites[125].alpha, 0);
  CHECK_EQ(gosteksprites[126].cx, 0.000000f);
  CHECK_EQ(gosteksprites[126].image, 13);
  CHECK_EQ(gosteksprites[126].p1, 10);
  CHECK_EQ(gosteksprites[126].p2, 13);
  CHECK_EQ(gosteksprites[126].id, "Right_Arm");
  CHECK_EQ(gosteksprites[126].cy, 0.600000f);
  CHECK_EQ(gosteksprites[126].flex, 0.000000f);
  CHECK_EQ(gosteksprites[126].flip, true);
  CHECK_EQ(gosteksprites[126].team, true);
  CHECK_EQ(gosteksprites[126].color, 1);
  CHECK_EQ(gosteksprites[126].alpha, 0);
  CHECK_EQ(gosteksprites[127].cx, -0.100000f);
  CHECK_EQ(gosteksprites[127].image, 50);
  CHECK_EQ(gosteksprites[127].p1, 10);
  CHECK_EQ(gosteksprites[127].p2, 13);
  CHECK_EQ(gosteksprites[127].id, "Right_Arm_Dmg");
  CHECK_EQ(gosteksprites[127].cy, 0.500000f);
  CHECK_EQ(gosteksprites[127].flex, 0.000000f);
  CHECK_EQ(gosteksprites[127].flip, true);
  CHECK_EQ(gosteksprites[127].team, true);
  CHECK_EQ(gosteksprites[127].color, 0);
  CHECK_EQ(gosteksprites[127].alpha, 1);
  CHECK_EQ(gosteksprites[128].cx, 0.000000f);
  CHECK_EQ(gosteksprites[128].image, 15);
  CHECK_EQ(gosteksprites[128].p1, 13);
  CHECK_EQ(gosteksprites[128].p2, 16);
  CHECK_EQ(gosteksprites[128].id, "Right_Forearm");
  CHECK_EQ(gosteksprites[128].cy, 0.600000f);
  CHECK_EQ(gosteksprites[128].flex, 5.000000f);
  CHECK_EQ(gosteksprites[128].flip, false);
  CHECK_EQ(gosteksprites[128].team, true);
  CHECK_EQ(gosteksprites[128].color, 1);
  CHECK_EQ(gosteksprites[128].alpha, 0);
  CHECK_EQ(gosteksprites[129].cx, 0.000000f);
  CHECK_EQ(gosteksprites[129].image, 52);
  CHECK_EQ(gosteksprites[129].p1, 13);
  CHECK_EQ(gosteksprites[129].p2, 16);
  CHECK_EQ(gosteksprites[129].id, "Right_Forearm_Dmg");
  CHECK_EQ(gosteksprites[129].cy, 0.600000f);
  CHECK_EQ(gosteksprites[129].flex, 5.000000f);
  CHECK_EQ(gosteksprites[129].flip, true);
  CHECK_EQ(gosteksprites[129].team, true);
  CHECK_EQ(gosteksprites[129].color, 0);
  CHECK_EQ(gosteksprites[129].alpha, 1);
  CHECK_EQ(gosteksprites[130].cx, 0.000000f);
  CHECK_EQ(gosteksprites[130].image, 17);
  CHECK_EQ(gosteksprites[130].p1, 16);
  CHECK_EQ(gosteksprites[130].p2, 20);
  CHECK_EQ(gosteksprites[130].id, "Right_Hand");
  CHECK_EQ(gosteksprites[130].cy, 0.500000f);
  CHECK_EQ(gosteksprites[130].flex, 0.000000f);
  CHECK_EQ(gosteksprites[130].flip, true);
  CHECK_EQ(gosteksprites[130].team, true);
  CHECK_EQ(gosteksprites[130].color, 3);
  CHECK_EQ(gosteksprites[130].alpha, 0);
}

void GostekGraphicsFixture::VerifyGostekBase(const tgostekspriteset &gostekbase)
{
  CHECK_FALSE(gostekbase.contains(0));
  CHECK_FALSE(gostekbase.contains(1));
  CHECK_FALSE(gostekbase.contains(2));
  CHECK_FALSE(gostekbase.contains(3));
  CHECK_FALSE(gostekbase.contains(4));
  CHECK_FALSE(gostekbase.contains(5));
  CHECK_FALSE(gostekbase.contains(6));
  CHECK_FALSE(gostekbase.contains(7));
  CHECK_FALSE(gostekbase.contains(8));
  CHECK_FALSE(gostekbase.contains(9));
  CHECK_FALSE(gostekbase.contains(10));
  CHECK_FALSE(gostekbase.contains(11));
  CHECK_FALSE(gostekbase.contains(12));
  CHECK_FALSE(gostekbase.contains(13));
  CHECK_FALSE(gostekbase.contains(14));
  CHECK_FALSE(gostekbase.contains(15));
  CHECK_FALSE(gostekbase.contains(16));
  CHECK(gostekbase.contains(17));
  CHECK_FALSE(gostekbase.contains(18));
  CHECK(gostekbase.contains(19));
  CHECK_FALSE(gostekbase.contains(20));
  CHECK(gostekbase.contains(21));
  CHECK_FALSE(gostekbase.contains(22));
  CHECK(gostekbase.contains(23));
  CHECK_FALSE(gostekbase.contains(24));
  CHECK(gostekbase.contains(25));
  CHECK_FALSE(gostekbase.contains(26));
  CHECK(gostekbase.contains(27));
  CHECK_FALSE(gostekbase.contains(28));
  CHECK_FALSE(gostekbase.contains(29));
  CHECK(gostekbase.contains(30));
  CHECK_FALSE(gostekbase.contains(31));
  CHECK(gostekbase.contains(32));
  CHECK_FALSE(gostekbase.contains(33));
  CHECK(gostekbase.contains(34));
  CHECK_FALSE(gostekbase.contains(35));
  CHECK(gostekbase.contains(36));
  CHECK_FALSE(gostekbase.contains(37));
  CHECK_FALSE(gostekbase.contains(38));
  CHECK(gostekbase.contains(39));
  CHECK_FALSE(gostekbase.contains(40));
  CHECK(gostekbase.contains(41));
  CHECK_FALSE(gostekbase.contains(42));
  CHECK_FALSE(gostekbase.contains(43));
  CHECK_FALSE(gostekbase.contains(44));
  CHECK_FALSE(gostekbase.contains(45));
  CHECK_FALSE(gostekbase.contains(46));
  CHECK_FALSE(gostekbase.contains(47));
  CHECK_FALSE(gostekbase.contains(48));
  CHECK_FALSE(gostekbase.contains(49));
  CHECK_FALSE(gostekbase.contains(50));
  CHECK_FALSE(gostekbase.contains(51));
  CHECK_FALSE(gostekbase.contains(52));
  CHECK_FALSE(gostekbase.contains(53));
  CHECK_FALSE(gostekbase.contains(54));
  CHECK_FALSE(gostekbase.contains(55));
  CHECK_FALSE(gostekbase.contains(56));
  CHECK_FALSE(gostekbase.contains(57));
  CHECK_FALSE(gostekbase.contains(58));
  CHECK_FALSE(gostekbase.contains(59));
  CHECK_FALSE(gostekbase.contains(60));
  CHECK_FALSE(gostekbase.contains(61));
  CHECK_FALSE(gostekbase.contains(62));
  CHECK_FALSE(gostekbase.contains(63));
  CHECK_FALSE(gostekbase.contains(64));
  CHECK_FALSE(gostekbase.contains(65));
  CHECK_FALSE(gostekbase.contains(66));
  CHECK_FALSE(gostekbase.contains(67));
  CHECK_FALSE(gostekbase.contains(68));
  CHECK_FALSE(gostekbase.contains(69));
  CHECK_FALSE(gostekbase.contains(70));
  CHECK_FALSE(gostekbase.contains(71));
  CHECK_FALSE(gostekbase.contains(72));
  CHECK_FALSE(gostekbase.contains(73));
  CHECK_FALSE(gostekbase.contains(74));
  CHECK_FALSE(gostekbase.contains(75));
  CHECK_FALSE(gostekbase.contains(76));
  CHECK_FALSE(gostekbase.contains(77));
  CHECK_FALSE(gostekbase.contains(78));
  CHECK_FALSE(gostekbase.contains(79));
  CHECK_FALSE(gostekbase.contains(80));
  CHECK_FALSE(gostekbase.contains(81));
  CHECK_FALSE(gostekbase.contains(82));
  CHECK_FALSE(gostekbase.contains(83));
  CHECK_FALSE(gostekbase.contains(84));
  CHECK_FALSE(gostekbase.contains(85));
  CHECK_FALSE(gostekbase.contains(86));
  CHECK_FALSE(gostekbase.contains(87));
  CHECK_FALSE(gostekbase.contains(88));
  CHECK_FALSE(gostekbase.contains(89));
  CHECK_FALSE(gostekbase.contains(90));
  CHECK_FALSE(gostekbase.contains(91));
  CHECK_FALSE(gostekbase.contains(92));
  CHECK_FALSE(gostekbase.contains(93));
  CHECK_FALSE(gostekbase.contains(94));
  CHECK_FALSE(gostekbase.contains(95));
  CHECK_FALSE(gostekbase.contains(96));
  CHECK_FALSE(gostekbase.contains(97));
  CHECK_FALSE(gostekbase.contains(98));
  CHECK_FALSE(gostekbase.contains(99));
  CHECK_FALSE(gostekbase.contains(100));
  CHECK_FALSE(gostekbase.contains(101));
  CHECK_FALSE(gostekbase.contains(102));
  CHECK_FALSE(gostekbase.contains(103));
  CHECK_FALSE(gostekbase.contains(104));
  CHECK_FALSE(gostekbase.contains(105));
  CHECK_FALSE(gostekbase.contains(106));
  CHECK_FALSE(gostekbase.contains(107));
  CHECK_FALSE(gostekbase.contains(108));
  CHECK_FALSE(gostekbase.contains(109));
  CHECK_FALSE(gostekbase.contains(110));
  CHECK_FALSE(gostekbase.contains(111));
  CHECK_FALSE(gostekbase.contains(112));
  CHECK_FALSE(gostekbase.contains(113));
  CHECK_FALSE(gostekbase.contains(114));
  CHECK_FALSE(gostekbase.contains(115));
  CHECK_FALSE(gostekbase.contains(116));
  CHECK_FALSE(gostekbase.contains(117));
  CHECK_FALSE(gostekbase.contains(118));
  CHECK_FALSE(gostekbase.contains(119));
  CHECK_FALSE(gostekbase.contains(120));
  CHECK_FALSE(gostekbase.contains(121));
  CHECK_FALSE(gostekbase.contains(122));
  CHECK_FALSE(gostekbase.contains(123));
  CHECK_FALSE(gostekbase.contains(124));
  CHECK_FALSE(gostekbase.contains(125));
  CHECK(gostekbase.contains(126));
  CHECK_FALSE(gostekbase.contains(127));
  CHECK(gostekbase.contains(128));
  CHECK_FALSE(gostekbase.contains(129));
  CHECK(gostekbase.contains(130));
}

// NOLINTEND