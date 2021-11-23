#include "shared/misc/GlobalVariable.hpp"
#include "common/misc/PortUtils.hpp"
#include "shared/misc/SoldatConfig.hpp"
#include <gtest/gtest.h>

namespace
{
auto &TestVariable = InitGlobalVariable<float, "TestVariable">();
}

TEST(GlobalVariableTest, MultipleCallsToInitGlobalVariableCauseTheSameVariableToInit)
{
    TestVariable = 9;
    auto &t = InitGlobalVariable<float, "TestVariable">();
    t = 12;
    auto &f = InitGlobalVariable<float, "TestVariable">();
    f = 14;

    EXPECT_EQ(14.0f, t);
    EXPECT_EQ(14.0f, f);
    EXPECT_EQ(14.0f, TestVariable);
}

TEST(GlobalVariableTest, DifferentModulessUseDifferentVariables)
{
    TestVariable = 9;
    auto &clientVar = InitGlobalVariable<float, "TestVariable", Config::CLIENT_MODULE>();
    clientVar = 12;
    auto &serverVar = InitGlobalVariable<float, "TestVariable", Config::SERVER_MODULE>();
    serverVar = 14;

    EXPECT_EQ(12.0f, clientVar);
    EXPECT_EQ(14.0f, serverVar);
    EXPECT_EQ(9.0f, TestVariable);
}
