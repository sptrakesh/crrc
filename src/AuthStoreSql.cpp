#include "AuthStoreSql.h"
#include "dao/UserDAO.h"

using crrc::AuthStoreSql;

AuthStoreSql::AuthStoreSql( QObject* parent ) : AuthenticationStore ( parent )
{
  idField = "username";
}

Cutelyst::AuthenticationUser AuthStoreSql::findUser( Cutelyst::Context* c,
  const Cutelyst::ParamsMultiMap& userinfo )
{
  const auto id = userinfo[idField];
  const auto user = dao::UserDAO().retrieveByUsername( id );

  if ( ! user.isEmpty() )
  {
    Cutelyst::AuthenticationUser u( user.value( "user_id" ).toString() );

    for ( auto it = user.constBegin(); it != user.constEnd(); ++it )
    {
      u.insert( it.key(), it.value() );
    }

    return u;
  }

  return Cutelyst::AuthenticationUser();
}
