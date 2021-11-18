#include "shared/misc/GlobalVariable.hpp"

template <typename T, PortUtils::StringLiteral VariableName, Config::Module M>
T &InitGlobalVariable()
{
    static T global = {};
    return global;
}

namespace
{
auto &TestVariable = InitGlobalVariable<float, "TestVariable">();
}