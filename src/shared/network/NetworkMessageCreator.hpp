#pragma once

#include <memory>
#include <boost/di/extension/providers/runtime_provider.hpp>

class INetwork;

class NetworkMessageCreator
{
public:
  NetworkMessageCreator(std::shared_ptr<INetwork> inetwork)
  : mNetwork(inetwork)
  {
  }

  template<typename T>
  inline T CreateMsg()
  {
    auto inj =
      boost::di::make_injector(boost::di::bind<INetwork>.to(
       [&] (const auto& injector) -> std::shared_ptr<INetwork>
          {
            return mNetwork;
          }
      ));
    return inj.template create<T>();
  }

private:
  std::shared_ptr<INetwork> mNetwork;
};