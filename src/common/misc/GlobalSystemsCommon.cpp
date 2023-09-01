#include "GlobalSystemsCommon.hpp"

GlobalSystemsCommon::GlobalSystemsCommon()
{
  FileUtilityObject = std::make_unique<FileUtility>();
}

GlobalSystemsCommon::~GlobalSystemsCommon()
{
  FileUtilityObject.reset();
}
