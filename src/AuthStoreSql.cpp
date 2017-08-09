#include "AuthStoreSql.h"
#include "constants.h"

#include <Cutelyst/Plugins/Utils/Sql>

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>

using crrc::AuthStoreSql;

AuthStoreSql::AuthStoreSql( QObject* parent ) : AuthenticationStore ( parent )
{
  idField = "username";
}

Cutelyst::AuthenticationUser AuthStoreSql::findUser( Cutelyst::Context* c,
  const Cutelyst::ParamsMultiMap& userinfo )
{
  auto id = userinfo[idField];

  auto query = CPreparedSqlQueryThreadForDB(
    "select * from users where username = :username", DATABASE_NAME );
  query.bindValue ( ":username", id );
  if ( query.exec() && query.next() )
  {
    auto userId = query.value ( "user_id" );
    qDebug() << "FOUND USER -> " << userId.toInt();
    Cutelyst::AuthenticationUser user ( userId.toString() );

    int columns = query.record().count();
    QStringList cols;
    for ( auto j = 0; j < columns; ++j ) cols << query.record().fieldName ( j );

    for ( auto j = 0; j < columns; ++j )
    {
      user.insert ( cols.at ( j ), query.value ( j ).toString() );
    }

    return user;
  }
  qDebug() << query.lastError().text();

  return Cutelyst::AuthenticationUser();
}
