#include "AuthStoreSql.h"
#include "dao/UserDAO.h"
#include "model/User.h"

#include <QtCore/QDebug>

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
  const auto uptr = qvariant_cast<model::User*>( user );

  if ( uptr )
  {
    Cutelyst::AuthenticationUser u( QString::number( uptr->getId() ) );
    u.insert( "id", uptr->getId() );
    u.insert( "username", uptr->getUsername() );
    u.insert( "password", uptr->getPassword() );
    u.insert( "firstName", uptr->getFirstName() );
    u.insert( "lastName", uptr->getLastName() );
    u.insert( "middleName", uptr->getMiddleName() );
    u.insert( "role", uptr->getRole() );

    return u;
  }

  return Cutelyst::AuthenticationUser();
}
