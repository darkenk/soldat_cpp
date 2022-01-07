#include "common/WeaponSystem.hpp"
#include "gtest/gtest.h"

TEST(WeaponSystemTest, CountEnabledWeapons)
{
    WeaponSystem ws;
    EXPECT_EQ(0, ws.CountEnabledPrimaryWeapons() + ws.CountEnabledSecondaryWeapons());

    ws.EnableAllWeapons();
    EXPECT_EQ(10, ws.CountEnabledPrimaryWeapons());
    EXPECT_EQ(4, ws.CountEnabledSecondaryWeapons());
    EXPECT_EQ(14, ws.CountEnabledPrimaryWeapons() + ws.CountEnabledSecondaryWeapons());

    ws.EnableWeapon(minigun, false);
    EXPECT_EQ(13, ws.CountEnabledPrimaryWeapons() + ws.CountEnabledSecondaryWeapons());

    ws.EnableWeapon(law, false);
    EXPECT_EQ(12, ws.CountEnabledPrimaryWeapons() + ws.CountEnabledSecondaryWeapons());
}

TEST(WeaponSystemTest, EnableWeaponTests)
{
    WeaponSystem ws;
    ws.EnableAllWeapons();
    EXPECT_TRUE(ws.IsEnabled(m79));
    ws.EnableWeapon(m79, false);
    EXPECT_FALSE(ws.IsEnabled(m79));
}
