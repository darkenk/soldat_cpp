#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_DEAGLES = 0;
#else
def("Secondary_Deagles", 0, 5, 10, 0.3, 0.5, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_MP5 = 1;
#else
def("Secondary_Mp5", GFX::WEAPONS_MP5, 5, 10, 0.3, 0.3, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_AK74 = 2;
#else
def("Secondary_Ak74", GFX::WEAPONS_AK74, 5, 10, 0.3, 0.25, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_STEYR = 3;
#else
def("Secondary_Steyr", GFX::WEAPONS_STEYR, 5, 10, 0.3, 0.5, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_SPAS = 4;
#else
def("Secondary_Spas", GFX::WEAPONS_SPAS, 5, 10, 0.3, 0.3, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_RUGER = 5;
#else
def("Secondary_Ruger", GFX::WEAPONS_RUGER, 5, 10, 0.3, 0.3, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_M79 = 6;
#else
def("Secondary_M79", GFX::WEAPONS_M79, 5, 10, 0.3, 0.35, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_BARRETT = 7;
#else
def("Secondary_Barrett", GFX::WEAPONS_BARRETT, 5, 10, 0.3, 0.35, 0, 1, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_MINIMI = 8;
#else
def("Secondary_Minimi", GFX::WEAPONS_MINIMI, 5, 10, 0.3, 0.35, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_MINIGUN = 9;
#else
def("Secondary_Minigun", GFX::WEAPONS_MINIGUN, 5, 10, 0.2, 0.5, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_SOCOM = 10;
#else
def("Secondary_Socom", 0, 5, 10, 0.3, 0.5, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_KNIFE = 11;
#else
def("Secondary_Knife", 0, 5, 10, 0.3, 0.5, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_CHAINSAW = 12;
#else
def("Secondary_Chainsaw", GFX::WEAPONS_CHAINSAW, 5, 10, 0.25, 0.5, 0, 1, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_LAW = 13;
#else
def("Secondary_Law", GFX::WEAPONS_LAW, 5, 10, 0.3, 0.45, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_FLAMEBOW = 14;
#else
def("Secondary_Flamebow", 0, 5, 10, 0.3, 0.5, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_BOW = 15;
#else
def("Secondary_Bow", 0, 5, 10, 0.3, 0.5, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SECONDARY_FLAMER = 16;
#else
def("Secondary_Flamer", GFX::WEAPONS_FLAMER, 5, 10, 0.3, 0.3, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_LEFT_THIGH = 17;
#else
def("Left_Thigh", GFX::GOSTEK_UDO, 6, 3, 0.2, 0.5, 1, 1, 1, 5, color_pants, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_LEFT_THIGH_DMG = 18;
#else
def("Left_Thigh_Dmg", GFX::GOSTEK_RANNY_UDO, 6, 3, 0.2, 0.5, 0, 1, 1, 5, color_none, alpha_blood);
#endif
#if IDS
constexpr std::int32_t GOSTEK_LEFT_FOOT = 19;
#else
def("Left_Foot", GFX::GOSTEK_STOPA, 2, 18, 0.35, 0.35, 1, 1, 1, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_LEFT_JETFOOT = 20;
#else
def("Left_Jetfoot", GFX::GOSTEK_LECISTOPA, 2, 18, 0.35, 0.35, 0, 1, 1, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_LEFT_LOWERLEG = 21;
#else
def("Left_Lowerleg", GFX::GOSTEK_NOGA, 3, 2, 0.15, 0.55, 1, 1, 1, 0, color_pants, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_LEFT_LOWERLEG_DMG = 22;
#else
def("Left_Lowerleg_Dmg", GFX::GOSTEK_RANNY_NOGA, 3, 2, 0.15, 0.55, 0, 1, 1, 0, color_none,
    alpha_blood);
#endif
#if IDS
constexpr std::int32_t GOSTEK_LEFT_ARM = 23;
#else
def("Left_Arm", GFX::GOSTEK_RAMIE, 11, 14, 0, 0.5, 1, 1, 1, 0, color_main, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_LEFT_ARM_DMG = 24;
#else
def("Left_Arm_Dmg", GFX::GOSTEK_RANNY_RAMIE, 11, 14, 0, 0.5, 0, 1, 1, 0, color_none, alpha_blood);
#endif
#if IDS
constexpr std::int32_t GOSTEK_LEFT_FOREARM = 25;
#else
def("Left_Forearm", GFX::GOSTEK_REKA, 14, 15, 0, 0.5, 1, 0, 1, 5, color_main, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_LEFT_FOREARM_DMG = 26;
#else
def("Left_Forearm_Dmg", GFX::GOSTEK_RANNY_REKA, 14, 15, 0, 0.5, 0, 1, 1, 5, color_none,
    alpha_blood);
#endif
#if IDS
constexpr std::int32_t GOSTEK_LEFT_HAND = 27;
#else
def("Left_Hand", GFX::GOSTEK_DLON, 15, 19, 0, 0.4, 1, 1, 1, 0, color_skin, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_GRABBED_HELMET = 28;
#else
def("Grabbed_Helmet", GFX::GOSTEK_HELM, 15, 19, 0, 0.5, 0, 1, 1, 0, color_main, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_GRABBED_HAT = 29;
#else
def("Grabbed_Hat", GFX::GOSTEK_KAP, 15, 19, 0.1, 0.4, 0, 1, 1, 0, color_main, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_RIGHT_THIGH = 30;
#else
def("Right_Thigh", GFX::GOSTEK_UDO, 5, 4, 0.2, 0.65, 1, 1, 1, 5, color_pants, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_RIGHT_THIGH_DMG = 31;
#else
def("Right_Thigh_Dmg", GFX::GOSTEK_RANNY_UDO, 5, 4, 0.2, 0.65, 0, 1, 1, 5, color_none, alpha_blood);
#endif
#if IDS
constexpr std::int32_t GOSTEK_RIGHT_FOOT = 32;
#else
def("Right_Foot", GFX::GOSTEK_STOPA, 1, 17, 0.35, 0.35, 1, 1, 1, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_RIGHT_JETFOOT = 33;
#else
def("Right_Jetfoot", GFX::GOSTEK_LECISTOPA, 1, 17, 0.35, 0.35, 0, 1, 1, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_RIGHT_LOWERLEG = 34;
#else
def("Right_Lowerleg", GFX::GOSTEK_NOGA, 4, 1, 0.15, 0.55, 1, 1, 1, 0, color_pants, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_RIGHT_LOWERLEG_DMG = 35;
#else
def("Right_Lowerleg_Dmg", GFX::GOSTEK_RANNY_NOGA, 4, 1, 0.15, 0.55, 0, 1, 1, 0, color_none,
    alpha_blood);
#endif
#if IDS
constexpr std::int32_t GOSTEK_CHEST = 36;
#else
def("Chest", GFX::GOSTEK_KLATA, 10, 11, 0.1, 0.3, 1, 1, 1, 0, color_main, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_VEST = 37;
#else
def("Vest", GFX::GOSTEK_KAMIZELKA, 10, 11, 0.1, 0.3, 0, 1, 1, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_CHEST_DMG = 38;
#else
def("Chest_Dmg", GFX::GOSTEK_RANNY_KLATA, 10, 11, 0.1, 0.3, 0, 1, 1, 0, color_none, alpha_blood);
#endif
#if IDS
constexpr std::int32_t GOSTEK_HIP = 39;
#else
def("Hip", GFX::GOSTEK_BIODRO, 5, 6, 0.25, 0.6, 1, 1, 1, 0, color_main, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_HIP_DMG = 40;
#else
def("Hip_Dmg", GFX::GOSTEK_RANNY_BIODRO, 5, 6, 0.25, 0.6, 0, 1, 1, 0, color_none, alpha_blood);
#endif
#if IDS
constexpr std::int32_t GOSTEK_HEAD = 41;
#else
def("Head", GFX::GOSTEK_MORDA, 9, 12, 0, 0.5, 1, 1, 1, 0, color_skin, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_HEAD_DMG = 42;
#else
def("Head_Dmg", GFX::GOSTEK_RANNY_MORDA, 9, 12, 0, 0.5, 0, 1, 1, 0, color_headblood, alpha_blood);
#endif
#if IDS
constexpr std::int32_t GOSTEK_HEAD_DEAD = 43;
#else
def("Head_Dead", GFX::GOSTEK_MORDA, 9, 12, 0.5, 0.5, 0, 1, 1, 0, color_skin, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_HEAD_DEAD_DMG = 44;
#else
def("Head_Dead_Dmg", GFX::GOSTEK_RANNY_MORDA, 9, 12, 0.5, 0.5, 0, 1, 1, 0, color_headblood,
    alpha_blood);
#endif
#if IDS
constexpr std::int32_t GOSTEK_MR_T = 45;
#else
def("Mr_T", GFX::GOSTEK_HAIR3, 9, 12, 0, 0.5, 0, 1, 1, 0, color_hair, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_HELMET = 46;
#else
def("Helmet", GFX::GOSTEK_HELM, 9, 12, 0, 0.5, 0, 1, 1, 0, color_main, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_HAT = 47;
#else
def("Hat", GFX::GOSTEK_KAP, 9, 12, 0, 0.5, 0, 1, 1, 0, color_main, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_RAMBO_BADGE = 48;
#else
def("Rambo_Badge", GFX::GOSTEK_BADGE, 9, 12, 0, 0.5, 0, 1, 1, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_HAIR_DREADLOCKS = 49;
#else
def("Hair_Dreadlocks", GFX::GOSTEK_HAIR1, 9, 12, 0, 0.5, 0, 1, 1, 0, color_hair, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_HAIR_DREADLOCK1 = 50;
#else
def("Hair_Dreadlock1", GFX::GOSTEK_DRED, 23, 24, 0, 1.22, 0, 0, 1, 0, color_hair, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_HAIR_DREADLOCK2 = 51;
#else
def("Hair_Dreadlock2", GFX::GOSTEK_DRED, 23, 24, 0.1, 0.5, 0, 0, 1, 0, color_hair, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_HAIR_DREADLOCK3 = 52;
#else
def("Hair_Dreadlock3", GFX::GOSTEK_DRED, 23, 24, 0.04, -0.3, 0, 0, 1, 0, color_hair, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_HAIR_DREADLOCK4 = 53;
#else
def("Hair_Dreadlock4", GFX::GOSTEK_DRED, 23, 24, 0, -0.9, 0, 0, 1, 0, color_hair, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_HAIR_DREADLOCK5 = 54;
#else
def("Hair_Dreadlock5", GFX::GOSTEK_DRED, 23, 24, -0.2, -1.35, 0, 0, 1, 0, color_hair, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_HAIR_PUNK = 55;
#else
def("Hair_Punk", GFX::GOSTEK_HAIR2, 9, 12, 0, 0.5, 0, 1, 1, 0, color_hair, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_HAIR_NORMAL = 56;
#else
def("Hair_Normal", GFX::GOSTEK_HAIR4, 9, 12, 0, 0.5, 0, 1, 1, 0, color_hair, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_CIGAR = 57;
#else
def("Cigar", GFX::GOSTEK_CYGARO, 9, 12, -0.125, 0.4, 0, 1, 1, 0, color_cygar, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SILVER_LCHAIN = 58;
#else
def("Silver_Lchain", GFX::GOSTEK_LANCUCH, 10, 22, 0.1, 0.5, 0, 0, 1, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SILVER_RCHAIN = 59;
#else
def("Silver_Rchain", GFX::GOSTEK_LANCUCH, 11, 22, 0.1, 0.5, 0, 0, 1, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_SILVER_PENDANT = 60;
#else
def("Silver_Pendant", GFX::GOSTEK_METAL, 22, 21, 0.5, 0.7, 0, 1, 1, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_GOLDEN_LCHAIN = 61;
#else
def("Golden_Lchain", GFX::GOSTEK_ZLOTYLANCUCH, 10, 22, 0.1, 0.5, 0, 0, 1, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_GOLDEN_RCHAIN = 62;
#else
def("Golden_Rchain", GFX::GOSTEK_ZLOTYLANCUCH, 11, 22, 0.1, 0.5, 0, 0, 1, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_GOLDEN_PENDANT = 63;
#else
def("Golden_Pendant", GFX::GOSTEK_ZLOTO, 22, 21, 0.5, 0.5, 0, 1, 1, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_FRAG_GRENADE1 = 64;
#else
def("Frag_Grenade1", GFX::WEAPONS_FRAG_GRENADE, 5, 6, 0.5, 0.1, 0, 0, 0, 0, color_none,
    alpha_nades);
#endif
#if IDS
constexpr std::int32_t GOSTEK_FRAG_GRENADE2 = 65;
#else
def("Frag_Grenade2", GFX::WEAPONS_FRAG_GRENADE, 5, 6, 0.5, 0.1, 0, 0, 0, 0, color_none,
    alpha_nades);
#endif
#if IDS
constexpr std::int32_t GOSTEK_FRAG_GRENADE3 = 66;
#else
def("Frag_Grenade3", GFX::WEAPONS_FRAG_GRENADE, 5, 6, 0.5, 0.1, 0, 0, 0, 0, color_none,
    alpha_nades);
#endif
#if IDS
constexpr std::int32_t GOSTEK_FRAG_GRENADE4 = 67;
#else
def("Frag_Grenade4", GFX::WEAPONS_FRAG_GRENADE, 5, 6, 0.5, 0.1, 0, 0, 0, 0, color_none,
    alpha_nades);
#endif
#if IDS
constexpr std::int32_t GOSTEK_FRAG_GRENADE5 = 68;
#else
def("Frag_Grenade5", GFX::WEAPONS_FRAG_GRENADE, 5, 6, 0.5, 0.1, 0, 0, 0, 0, color_none,
    alpha_nades);
#endif
#if IDS
constexpr std::int32_t GOSTEK_CLUSTER_GRENADE1 = 69;
#else
def("Cluster_Grenade1", GFX::WEAPONS_CLUSTER_GRENADE, 5, 6, 0.5, 0.3, 0, 0, 0, 0, color_none,
    alpha_nades);
#endif
#if IDS
constexpr std::int32_t GOSTEK_CLUSTER_GRENADE2 = 70;
#else
def("Cluster_Grenade2", GFX::WEAPONS_CLUSTER_GRENADE, 5, 6, 0.5, 0.3, 0, 0, 0, 0, color_none,
    alpha_nades);
#endif
#if IDS
constexpr std::int32_t GOSTEK_CLUSTER_GRENADE3 = 71;
#else
def("Cluster_Grenade3", GFX::WEAPONS_CLUSTER_GRENADE, 5, 6, 0.5, 0.3, 0, 0, 0, 0, color_none,
    alpha_nades);
#endif
#if IDS
constexpr std::int32_t GOSTEK_CLUSTER_GRENADE4 = 72;
#else
def("Cluster_Grenade4", GFX::WEAPONS_CLUSTER_GRENADE, 5, 6, 0.5, 0.3, 0, 0, 0, 0, color_none,
    alpha_nades);
#endif
#if IDS
constexpr std::int32_t GOSTEK_CLUSTER_GRENADE5 = 73;
#else
def("Cluster_Grenade5", GFX::WEAPONS_CLUSTER_GRENADE, 5, 6, 0.5, 0.3, 0, 0, 0, 0, color_none,
    alpha_nades);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_DEAGLES = 74;
#else
def("Primary_Deagles", GFX::WEAPONS_DEAGLES, 16, 15, 0.1, 0.8, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_DEAGLES_CLIP = 75;
#else
def("Primary_Deagles_Clip", GFX::WEAPONS_DEAGLES_CLIP, 16, 15, 0.1, 0.8, 0, 1, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_DEAGLES_FIRE = 76;
#else
def("Primary_Deagles_Fire", GFX::WEAPONS_DEAGLES_FIRE, 16, 15, -0.5, 1, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_MP5 = 77;
#else
def("Primary_Mp5", GFX::WEAPONS_MP5, 16, 15, 0.15, 0.6, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_MP5_CLIP = 78;
#else
def("Primary_Mp5_Clip", GFX::WEAPONS_MP5_CLIP, 16, 15, 0.15, 0.6, 0, 1, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_MP5_FIRE = 79;
#else
def("Primary_Mp5_Fire", GFX::WEAPONS_MP5_FIRE, 16, 15, -0.65, 0.85, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_AK74 = 80;
#else
def("Primary_Ak74", GFX::WEAPONS_AK74, 16, 15, 0.15, 0.5, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_AK74_CLIP = 81;
#else
def("Primary_Ak74_Clip", GFX::WEAPONS_AK74_CLIP, 16, 15, 0.15, 0.5, 0, 1, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_AK74_FIRE = 82;
#else
def("Primary_Ak74_Fire", GFX::WEAPONS_AK74_FIRE, 16, 15, -0.37, 0.8, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_STEYR = 83;
#else
def("Primary_Steyr", GFX::WEAPONS_STEYR, 16, 15, 0.2, 0.6, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_STEYR_CLIP = 84;
#else
def("Primary_Steyr_Clip", GFX::WEAPONS_STEYR_CLIP, 16, 15, 0.2, 0.6, 0, 1, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_STEYR_FIRE = 85;
#else
def("Primary_Steyr_Fire", GFX::WEAPONS_STEYR_FIRE, 16, 15, -0.24, 0.75, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_SPAS = 86;
#else
def("Primary_Spas", GFX::WEAPONS_SPAS, 16, 15, 0.1, 0.6, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_SPAS_CLIP = 87;
#else
def("Primary_Spas_Clip", 0, 16, 15, 0, 0, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_SPAS_FIRE = 88;
#else
def("Primary_Spas_Fire", GFX::WEAPONS_SPAS_FIRE, 16, 15, -0.2, 0.9, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_RUGER = 89;
#else
def("Primary_Ruger", GFX::WEAPONS_RUGER, 16, 15, 0.1, 0.7, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_RUGER_CLIP = 90;
#else
def("Primary_Ruger_Clip", 0, 16, 15, 0, 0, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_RUGER_FIRE = 91;
#else
def("Primary_Ruger_Fire", GFX::WEAPONS_RUGER_FIRE, 16, 15, -0.35, 0.85, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_M79 = 92;
#else
def("Primary_M79", GFX::WEAPONS_M79, 16, 15, 0.1, 0.7, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_M79_CLIP = 93;
#else
def("Primary_M79_Clip", GFX::WEAPONS_M79_CLIP, 16, 15, 0.1, 0.7, 0, 1, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_M79_FIRE = 94;
#else
def("Primary_M79_Fire", GFX::WEAPONS_M79_FIRE, 16, 15, -0.4, 0.8, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_BARRETT = 95;
#else
def("Primary_Barrett", GFX::WEAPONS_BARRETT, 16, 15, 0.15, 0.7, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_BARRETT_CLIP = 96;
#else
def("Primary_Barrett_Clip", GFX::WEAPONS_BARRETT_CLIP, 16, 15, 0.15, 0.7, 0, 1, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_BARRETT_FIRE = 97;
#else
def("Primary_Barrett_Fire", GFX::WEAPONS_BARRETT_FIRE, 16, 15, -0.15, 0.8, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_MINIMI = 98;
#else
def("Primary_Minimi", GFX::WEAPONS_MINIMI, 16, 15, 0.15, 0.6, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_MINIMI_CLIP = 99;
#else
def("Primary_Minimi_Clip", GFX::WEAPONS_MINIMI_CLIP, 16, 15, 0.15, 0.6, 0, 1, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_MINIMI_FIRE = 100;
#else
def("Primary_Minimi_Fire", GFX::WEAPONS_MINIMI_FIRE, 16, 15, -0.2, 0.9, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_MINIGUN_CLIP = 101;
#else
def("Primary_Minigun_Clip", GFX::WEAPONS_MINIGUN_CLIP, 8, 7, 0.5, 0.1, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_MINIGUN = 102;
#else
def("Primary_Minigun", GFX::WEAPONS_MINIGUN, 16, 15, 0.05, 0.5, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_MINIGUN_FIRE = 103;
#else
def("Primary_Minigun_Fire", GFX::WEAPONS_MINIGUN_FIRE, 16, 15, -0.2, 0.45, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_SOCOM = 104;
#else
def("Primary_Socom", GFX::WEAPONS_SOCOM, 16, 15, 0.2, 0.55, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_SOCOM_CLIP = 105;
#else
def("Primary_Socom_Clip", GFX::WEAPONS_SOCOM_CLIP, 16, 15, 0.2, 0.55, 0, 1, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_SOCOM_FIRE = 106;
#else
def("Primary_Socom_Fire", GFX::WEAPONS_SOCOM_FIRE, 16, 15, -0.24, 0.85, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_KNIFE = 107;
#else
def("Primary_Knife", GFX::WEAPONS_KNIFE, 16, 20, -0.1, 0.6, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_KNIFE_CLIP = 108;
#else
def("Primary_Knife_Clip", 0, 16, 15, 0, 0, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_KNIFE_FIRE = 109;
#else
def("Primary_Knife_Fire", 0, 16, 15, 0, 0, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_CHAINSAW = 110;
#else
def("Primary_Chainsaw", GFX::WEAPONS_CHAINSAW, 16, 15, 0.1, 0.5, 0, 1, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_CHAINSAW_CLIP = 111;
#else
def("Primary_Chainsaw_Clip", 0, 16, 15, 0, 0, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_CHAINSAW_FIRE = 112;
#else
def("Primary_Chainsaw_Fire", GFX::WEAPONS_CHAINSAW_FIRE, 16, 15, 0, 0, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_LAW = 113;
#else
def("Primary_Law", GFX::WEAPONS_LAW, 16, 15, 0.1, 0.6, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_LAW_CLIP = 114;
#else
def("Primary_Law_Clip", 0, 16, 15, 0, 0, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_LAW_FIRE = 115;
#else
def("Primary_Law_Fire", GFX::WEAPONS_LAW_FIRE, 16, 15, -0.1, 0.55, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_BOW = 116;
#else
def("Primary_Bow", GFX::WEAPONS_BOW, 16, 15, -0.4, 0.55, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_BOW_ARROW = 117;
#else
def("Primary_Bow_Arrow", GFX::WEAPONS_BOW_A, 16, 15, 0, 0.55, 0, 0, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_BOW_STRING = 118;
#else
def("Primary_Bow_String", GFX::WEAPONS_BOW_S, 16, 15, -0.4, 0.55, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_BOW_RELOAD = 119;
#else
def("Primary_Bow_Reload", GFX::WEAPONS_BOW, 16, 15, -0.4, 0.55, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_BOW_ARROW_RELOAD = 120;
#else
def("Primary_Bow_Arrow_Reload", GFX::WEAPONS_ARROW, 16, 20, 0, 0.55, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_BOW_STRING_RELOAD = 121;
#else
def("Primary_Bow_String_Reload", GFX::WEAPONS_BOW_S, 16, 15, -0.4, 0.55, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_BOW_FIRE = 122;
#else
def("Primary_Bow_Fire", GFX::WEAPONS_BOW_FIRE, 16, 15, 0, 0, 0, 0, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_FLAMER = 123;
#else
def("Primary_Flamer", GFX::WEAPONS_FLAMER, 16, 15, 0.2, 0.7, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_FLAMER_CLIP = 124;
#else
def("Primary_Flamer_Clip", 0, 16, 15, 0, 0, 0, 1, 0, 0, color_none, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_PRIMARY_FLAMER_FIRE = 125;
#else
def("Primary_Flamer_Fire", GFX::WEAPONS_FLAMER_FIRE, 16, 15, 0, 0, 0, 0, 0, 0, color_none,
    alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_RIGHT_ARM = 126;
#else
def("Right_Arm", GFX::GOSTEK_RAMIE, 10, 13, 0, 0.6, 1, 1, 1, 0, color_main, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_RIGHT_ARM_DMG = 127;
#else
def("Right_Arm_Dmg", GFX::GOSTEK_RANNY_RAMIE, 10, 13, -0.1, 0.5, 0, 1, 1, 0, color_none,
    alpha_blood);
#endif
#if IDS
constexpr std::int32_t GOSTEK_RIGHT_FOREARM = 128;
#else
def("Right_Forearm", GFX::GOSTEK_REKA, 13, 16, 0, 0.6, 1, 0, 1, 5, color_main, alpha_base);
#endif
#if IDS
constexpr std::int32_t GOSTEK_RIGHT_FOREARM_DMG = 129;
#else
def("Right_Forearm_Dmg", GFX::GOSTEK_RANNY_REKA, 13, 16, 0, 0.6, 0, 1, 1, 5, color_none,
    alpha_blood);
#endif
#if IDS
constexpr std::int32_t GOSTEK_RIGHT_HAND = 130;
#else
def("Right_Hand", GFX::GOSTEK_DLON, 16, 20, 0, 0.5, 1, 1, 1, 0, color_skin, alpha_base);
#endif

#if IDS
constexpr std::int32_t GOSTEK_FIRST = GOSTEK_SECONDARY_DEAGLES;
constexpr std::int32_t GOSTEK_LAST = GOSTEK_RIGHT_HAND + 1;
constexpr std::int32_t GOSTEK_PRIMARY_FIRST = GOSTEK_PRIMARY_DEAGLES;
constexpr std::int32_t GOSTEK_SECONDARY_FIRST = GOSTEK_SECONDARY_DEAGLES;

#endif
