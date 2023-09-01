#pragma once

#include "../FileUtility.hpp"
#include "GlobalSubsystem.hpp"
#include <memory>

class GlobalSystemsCommon final : public GlobalSubsystem<GlobalSystemsCommon>
{
public:
  static FileUtility &GetFileSystem()
  {
    return *GlobalSystemsCommon::Get().FileUtilityObject;
  }

protected:
  GlobalSystemsCommon();
  ~GlobalSystemsCommon();

private:
  friend std::default_delete<GlobalSystemsCommon>;
  friend GlobalSubsystem<GlobalSystemsCommon>;

  std::unique_ptr<FileUtility> FileUtilityObject;
};

using GSC = GlobalSystemsCommon;
