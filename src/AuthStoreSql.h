#pragma once
#include <Cutelyst/Plugins/Authentication/authenticationstore.h>


namespace crrc
{
  class AuthStoreSql : public Cutelyst::AuthenticationStore
  {
  public:
    explicit AuthStoreSql( QObject* parent = nullptr );

    virtual Cutelyst::AuthenticationUser findUser(
      Cutelyst::Context* c, const Cutelyst::ParamsMultiMap& userinfo );

  private:
    QString idField;
  };
}
