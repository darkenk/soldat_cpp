#include "shared/misc/GlobalVariable.hpp"

template <typename T, PortUtils::StringLiteral VariableName, Config::Module M, typename... Args>
T &InitGlobalVariable(Args &&...args)
{
    static T global = {std::forward<Args>(args)...};
    return global;
}

namespace
{
auto &TestVariable = InitGlobalVariable<float, "TestVariable">();
}
