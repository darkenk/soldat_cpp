#include "common/PolyMap.hpp"
#include "common/Constants.hpp"
#include <gtest/gtest.h>

bool ShouldTestPolygonWithRay(const std::uint8_t &polygonType, const bool &npcol, const bool &nbcol,
                              const bool &flag, const std::uint8_t &team)
{
    switch (polygonType)
    {
    case poly_type_normal:
        return true;
    case poly_type_only_bullets:
        return !nbcol;
    case poly_type_only_player:
        return !npcol;
    case poly_type_doesnt:
    case poly_type_background:
    case poly_type_background_transition:
        return false;
    case poly_type_red_bullets:
        return !((team != Constants::TEAM_ALPHA) || nbcol);
    case poly_type_red_player:
        return !((team != Constants::TEAM_ALPHA) || npcol);
    case poly_type_blue_bullets:
        return !((team != Constants::TEAM_BRAVO) || nbcol);
    case poly_type_blue_player:
        return !((team != Constants::TEAM_BRAVO) || npcol);
    case poly_type_yellow_bullets:
        return !((team != Constants::TEAM_CHARLIE) || nbcol);
    case poly_type_yellow_player:
        return !((team != Constants::TEAM_CHARLIE) || npcol);
    case poly_type_green_bullets:
        return !((team != Constants::TEAM_DELTA) || nbcol);
    case poly_type_green_player:
        return !((team != Constants::TEAM_DELTA) || npcol);
    case poly_type_only_flaggers:
        return !(!flag || npcol);
    case poly_type_not_flaggers:
        return !(flag || npcol);
    case poly_type_non_flagger_collides:
        return !(!flag || npcol || nbcol);
    default:;
    }
    return true;
}

TEST(PolyMapTest, TestSkipRayCastForSector)
{
    // clang-format off
    EXPECT_TRUE(ShouldTestPolygonWithRay(poly_type_normal, false, false, false, 0));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_only_bullets, false, true, false, 0));
    EXPECT_TRUE(ShouldTestPolygonWithRay(poly_type_only_bullets, false, false, false, 0));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_only_player, true, false, false, 0));
    EXPECT_TRUE(ShouldTestPolygonWithRay(poly_type_only_player, false, false, false, 0));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_doesnt, false, false, false, 0));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_background, false, false, false, 0));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_background_transition, false, false, false, 0));

    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_red_bullets, false, true, false, Constants::TEAM_ALPHA));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_red_bullets, false, false, false, Constants::TEAM_BRAVO));
    EXPECT_TRUE(ShouldTestPolygonWithRay(poly_type_red_bullets, false, false, false, Constants::TEAM_ALPHA));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_red_player, true, false, false, Constants::TEAM_ALPHA));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_red_player, false, false, false, Constants::TEAM_BRAVO));
    EXPECT_TRUE(ShouldTestPolygonWithRay(poly_type_red_player, false, false, false, Constants::TEAM_ALPHA));

    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_blue_bullets, false, true, false, Constants::TEAM_BRAVO));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_blue_bullets, false, false, false, Constants::TEAM_ALPHA));
    EXPECT_TRUE(ShouldTestPolygonWithRay(poly_type_blue_bullets, false, false, false, Constants::TEAM_BRAVO));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_blue_player, true, false, false, Constants::TEAM_BRAVO));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_blue_player, false, false, false, Constants::TEAM_ALPHA));
    EXPECT_TRUE(ShouldTestPolygonWithRay(poly_type_blue_player, false, false, false, Constants::TEAM_BRAVO));

    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_yellow_bullets, false, true, false, Constants::TEAM_CHARLIE));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_yellow_bullets, false, false, false, Constants::TEAM_ALPHA));
    EXPECT_TRUE(ShouldTestPolygonWithRay(poly_type_yellow_bullets, false, false, false, Constants::TEAM_CHARLIE));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_yellow_player, true, false, false, Constants::TEAM_CHARLIE));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_yellow_player, false, false, false, Constants::TEAM_ALPHA));
    EXPECT_TRUE(ShouldTestPolygonWithRay(poly_type_yellow_player, false, false, false, Constants::TEAM_CHARLIE));

    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_green_bullets, false, true, false, Constants::TEAM_DELTA));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_green_bullets, false, false, false, Constants::TEAM_ALPHA));
    EXPECT_TRUE(ShouldTestPolygonWithRay(poly_type_green_bullets, false, false, false, Constants::TEAM_DELTA));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_green_player, true, false, false, Constants::TEAM_DELTA));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_green_player, false, false, false, Constants::TEAM_ALPHA));
    EXPECT_TRUE(ShouldTestPolygonWithRay(poly_type_green_player, false, false, false, Constants::TEAM_DELTA));

    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_only_flaggers, true, false, false, 0));
    EXPECT_TRUE(ShouldTestPolygonWithRay(poly_type_only_flaggers, false, false, true, 0));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_only_flaggers, false, false, false, 0));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_not_flaggers, true, false, false, 0));
    EXPECT_TRUE(ShouldTestPolygonWithRay(poly_type_not_flaggers, false, false, false, 0));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_not_flaggers, false, false, true, 0));

    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_non_flagger_collides, false, false, false, 0));
    EXPECT_TRUE(ShouldTestPolygonWithRay(poly_type_non_flagger_collides, false, false, true, 0));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_non_flagger_collides, true, false, true, 0));
    EXPECT_FALSE(ShouldTestPolygonWithRay(poly_type_non_flagger_collides, false, true, true, 0));

    // clang-format on
}
