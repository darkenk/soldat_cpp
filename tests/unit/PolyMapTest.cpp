#include "common/PolyMap.hpp"
#include "common/Constants.hpp"
#include <gtest/gtest.h>

TEST(PolyMapTest, TestSkipRayCastForSector)
{
    // clang-format off
    EXPECT_TRUE(Polymap::ShouldTestPolygonWithRay(poly_type_normal, false, false, false, 0));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_only_bullets, false, true, false, 0));
    EXPECT_TRUE(Polymap::ShouldTestPolygonWithRay(poly_type_only_bullets, false, false, false, 0));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_only_player, true, false, false, 0));
    EXPECT_TRUE(Polymap::ShouldTestPolygonWithRay(poly_type_only_player, false, false, false, 0));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_doesnt, false, false, false, 0));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_background, false, false, false, 0));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_background_transition, false, false, false, 0));

    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_red_bullets, false, true, false, Constants::TEAM_ALPHA));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_red_bullets, false, false, false, Constants::TEAM_BRAVO));
    EXPECT_TRUE(Polymap::ShouldTestPolygonWithRay(poly_type_red_bullets, false, false, false, Constants::TEAM_ALPHA));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_red_player, true, false, false, Constants::TEAM_ALPHA));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_red_player, false, false, false, Constants::TEAM_BRAVO));
    EXPECT_TRUE(Polymap::ShouldTestPolygonWithRay(poly_type_red_player, false, false, false, Constants::TEAM_ALPHA));

    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_blue_bullets, false, true, false, Constants::TEAM_BRAVO));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_blue_bullets, false, false, false, Constants::TEAM_ALPHA));
    EXPECT_TRUE(Polymap::ShouldTestPolygonWithRay(poly_type_blue_bullets, false, false, false, Constants::TEAM_BRAVO));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_blue_player, true, false, false, Constants::TEAM_BRAVO));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_blue_player, false, false, false, Constants::TEAM_ALPHA));
    EXPECT_TRUE(Polymap::ShouldTestPolygonWithRay(poly_type_blue_player, false, false, false, Constants::TEAM_BRAVO));

    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_yellow_bullets, false, true, false, Constants::TEAM_CHARLIE));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_yellow_bullets, false, false, false, Constants::TEAM_ALPHA));
    EXPECT_TRUE(Polymap::ShouldTestPolygonWithRay(poly_type_yellow_bullets, false, false, false, Constants::TEAM_CHARLIE));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_yellow_player, true, false, false, Constants::TEAM_CHARLIE));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_yellow_player, false, false, false, Constants::TEAM_ALPHA));
    EXPECT_TRUE(Polymap::ShouldTestPolygonWithRay(poly_type_yellow_player, false, false, false, Constants::TEAM_CHARLIE));

    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_green_bullets, false, true, false, Constants::TEAM_DELTA));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_green_bullets, false, false, false, Constants::TEAM_ALPHA));
    EXPECT_TRUE(Polymap::ShouldTestPolygonWithRay(poly_type_green_bullets, false, false, false, Constants::TEAM_DELTA));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_green_player, true, false, false, Constants::TEAM_DELTA));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_green_player, false, false, false, Constants::TEAM_ALPHA));
    EXPECT_TRUE(Polymap::ShouldTestPolygonWithRay(poly_type_green_player, false, false, false, Constants::TEAM_DELTA));

    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_only_flaggers, true, false, false, 0));
    EXPECT_TRUE(Polymap::ShouldTestPolygonWithRay(poly_type_only_flaggers, false, false, true, 0));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_only_flaggers, false, false, false, 0));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_not_flaggers, true, false, false, 0));
    EXPECT_TRUE(Polymap::ShouldTestPolygonWithRay(poly_type_not_flaggers, false, false, false, 0));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_not_flaggers, false, false, true, 0));

    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_non_flagger_collides, false, false, false, 0));
    EXPECT_TRUE(Polymap::ShouldTestPolygonWithRay(poly_type_non_flagger_collides, false, false, true, 0));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_non_flagger_collides, true, false, true, 0));
    EXPECT_FALSE(Polymap::ShouldTestPolygonWithRay(poly_type_non_flagger_collides, false, true, true, 0));

    // clang-format on
}

TEST(PolyMapTest, GetSectorCoordInitialTest)
{
    twaypoints w;
    Polymap p{w};
    p.sectorsnum = 25;
    p.SetSectorsDivision(58);

    {
        auto coord = p.GetSectorCoord(tvector2(-100.f, 100.f));
        EXPECT_EQ(-2, coord.x);
        EXPECT_EQ(2, coord.y);
    }

    {
        auto coord = p.GetSectorCoord(tvector2(-58.f, 58.f));
        EXPECT_EQ(-1, coord.x);
        EXPECT_EQ(1, coord.y);
    }

    {
        auto coord = p.GetSectorCoord(tvector2(-28.f, 28.f));
        EXPECT_EQ(0, coord.x);
        EXPECT_EQ(0, coord.y);
    }

    {
        auto coord = p.GetSectorCoord(tvector2(-29.f, 29.f));
        EXPECT_EQ(-1, coord.x);
        EXPECT_EQ(1, coord.y);
    }

    {
        auto coord = p.GetSectorCoord(tvector2(-28.f - 58.f, 28.f + 58.f));
        EXPECT_EQ(-1, coord.x);
        EXPECT_EQ(1, coord.y);
    }

    {
        auto coord = p.GetSectorCoord(tvector2(-29.f - 58.f, 29.f + 58.f));
        EXPECT_EQ(-2, coord.x);
        EXPECT_EQ(2, coord.y);
    }
}

TEST(PolyMapTest, GetSectorCoordReturnsInvalidValueIfSectorDoesNotMatch)
{
    twaypoints w;
    Polymap p{w};
    p.sectorsnum = 25;
    p.SetSectorsDivision(58);

    {
        auto coord = p.GetSectorCoord(tvector2(58.0f * 25 + 29.0f, 0.0f));
        EXPECT_EQ(false, coord.IsValid());
    }

    {
        auto coord = p.GetSectorCoord(tvector2(58.0f * 25 + 28.0f, 0.0f));
        EXPECT_EQ(true, coord.IsValid());
        EXPECT_EQ(25, coord.x);
    }

    {
        auto coord = p.GetSectorCoord(tvector2(58.0f * 25 + 28.5f, 0.0f));
        EXPECT_EQ(true, coord.IsValid());
        EXPECT_EQ(25, coord.x);
    }
}

TEST(PolyMapTest, GetSectorPolygons)
{
    twaypoints w;
    Polymap p{w};
    tmapfile mapfile;
    mapfile.sectorsnum = 2;
    mapfile.sectorsdivision = 10;
    auto amountOfSectors = std::pow(2 * mapfile.sectorsnum + 1, 2);
    for (auto s = 0; s < amountOfSectors; s++)
    {
        mapfile.sectors.emplace_back().Polys.emplace_back(s);
    }
    p.loadmap(mapfile);

    {
        const auto sector = p.GetSector({-25.0f, -25.0f});
        EXPECT_EQ(false, sector.IsValid());
    }

    {
        const auto sector = p.GetSector({-15.0f, -15.0f});
        EXPECT_EQ(true, sector.IsValid());
        EXPECT_EQ(0, sector.GetPolys()[0]);
    }

    {
        const auto sector = p.GetSector({15.0f, 15.0f});
        EXPECT_EQ(true, sector.IsValid());
        EXPECT_EQ(24, sector.GetPolys()[0]);
    }
}
