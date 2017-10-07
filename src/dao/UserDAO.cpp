#include "UserDAO.h"
#include "RoleDAO.h"
#include "constants.h"
#include "model/User.h"
#include "model/Role.h"

#include <mutex>
#include <unordered_map>

#include <QtCore/QCryptographicHash>
#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>

using crrc::model::User;

namespace crrc
{
  namespace dao
  {
    static std::unordered_map<uint32_t, User::Ptr> users;
    static std::atomic_bool usersLoaded{ false };
    static std::mutex userMutex;

    QVariantList fromUsers()
    {
      QVariantList list;
      for ( const auto& iter : users )
      {
        list << asVariant( iter.second.get() );
      }

      return list;
    }

    void loadUsers()
    {
      if ( usersLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ userMutex };
      if ( !users.empty() )
      {
        usersLoaded = true;
        return;
      }

      auto query = CPreparedSqlQueryThreadForDB(
        "select user_id, username, password, email, first_name, last_name, middle_name, role_id from users order by username",
        DATABASE_NAME );

      if ( query.exec() )
      {
        while ( query.next() )
        {
          auto user = User::create( query );
          auto id = user->getId();
          users[id] = std::move( user );
        }

        usersLoaded = true;
      }
    }

    void bindUser( Cutelyst::Context* c, QSqlQuery& query )
    {
      query.bindValue( ":un", c->request()->param( "username" ) );

      QCryptographicHash hash{ QCryptographicHash::Sha3_512 };
      hash.addData( c->request()->param( "password" ).toLocal8Bit() );
      const auto passwd = hash.result().toHex();
      query.bindValue( ":passwd", passwd );

      query.bindValue( ":email", c->request()->param( "email" ) );
      query.bindValue( ":fn", c->request()->param( "firstName" ) );
      query.bindValue( ":ln", c->request()->param( "lastName" ) );
      query.bindValue( ":mn", c->request()->param( "middleName" ) );

      const auto role = c->request()->param( "role" );
      if ( ! role.isEmpty() && role != "-1" )
      {
        const auto r = RoleDAO().retrieve( role.toUInt() );
        const auto* rptr = model::Role::from( r );
        query.bindValue( ":role", rptr->getId() );
      }
      else query.bindValue( ":role", QVariant( QVariant::UInt ) );
    }
  }
}

using crrc::dao::UserDAO;

QVariantList UserDAO::retrieveAll() const
{
  loadUsers();
  return fromUsers();
}

QVariant UserDAO::retrieve( const uint32_t id ) const
{
  loadUsers();
  const auto iter = users.find( id );

  return ( iter != users.end() ) ? asVariant( iter->second.get() ) : QVariant();
}

QVariant UserDAO::retrieveByUsername( const QString& username ) const
{
  loadUsers();
  for ( const auto& iter : users )
  {
    if ( iter.second->getUsername() == username ) return asVariant( iter.second.get() );
  }

  return QVariantHash();
}

uint32_t UserDAO::insert( Cutelyst::Context* context ) const
{
  loadUsers();
  QSqlQuery query = CPreparedSqlQueryThreadForDB(
    "insert into users (username, password, email, first_name, last_name, middle_name, role_id) values (:un, :passwd, :email, :fn, :ln, :mn, :roleId)",
    crrc::DATABASE_NAME );
  bindUser( context, query );

  if ( !query.exec() )
  {
    context->stash()["error_msg"] = query.lastError().text();
    return 0;
  }

  const auto id = query.lastInsertId().toUInt();
  auto user = User::create( context );
  user->setId( id );
  std::lock_guard<std::mutex> lock{ userMutex };
  users[id] = std::move( user );
  return id;
}

void UserDAO::update( Cutelyst::Context* context ) const
{
  loadUsers();
  auto id = context->request()->param( "id" );
  auto query = CPreparedSqlQueryThreadForDB(
    "update users set username=:un, password=:passwd, email=:email, first_name=:fn, last_name=:ln, middle_name=:mn, role_id = :roleId where user_id=:id",
    crrc::DATABASE_NAME );
  bindUser( context, query );
  query.bindValue( ":id", id.toUInt() );

  if ( query.exec() )
  {
    if ( query.numRowsAffected() )
    {
      auto user = User::create( context );
      std::lock_guard<std::mutex> lock{ userMutex };
      users[id.toUInt()] = std::move( user );
    }

    context->stash()["count"] = query.numRowsAffected();
  }
  else context->stash()["error_msg"] = query.lastError().text();
}

QVariantList UserDAO::search( Cutelyst::Context* context ) const
{
  loadUsers();
  const auto text = context->request()->param( "text", "" );
  const QString clause = "%" % text % "%";

  auto query = CPreparedSqlQueryThreadForDB(
   "select user_id from users where first_name like :text or last_name like :text or username like :text order by name",
    DATABASE_NAME );

  query.bindValue( ":text", clause );
  QVariantList list;

  if ( query.exec() )
  {
    while ( query.next() )
    {
      const auto& user = retrieve( query.value( 0 ).toUInt() );
      list << user;
    }
  }
  else
  {
    qWarning() << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
  }

  return list;
}

uint32_t UserDAO::remove( uint32_t id ) const
{
  loadUsers();
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from users where user_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() )
  {
    std::lock_guard<std::mutex> lock{ userMutex };
    users.erase( id );
    return query.numRowsAffected();
  }

  qDebug() << query.lastError().text();
  return 0;
}

bool UserDAO::isUsernameAvailable( const QString& username ) const
{
  loadUsers();

  for ( const auto& iter : users )
  {
    if ( iter.second->getUsername() == username ) return false;
  }

  return true;
}

bool UserDAO::updateRole( uint32_t userId, uint32_t roleId ) const
{
  loadUsers();

  auto query = CPreparedSqlQueryThreadForDB(
   "update users set role_id = :roleId where user_id = :userId", DATABASE_NAME );
  query.bindValue( ":roleId", roleId ? roleId : QVariant( QVariant::UInt ) );
  query.bindValue( ":userId", userId );
  const auto result = query.exec() && query.numRowsAffected();

  if ( result )
  {
    auto iter = users.find( userId );
    std::lock_guard<std::mutex> lock{ userMutex };
    iter->second->setRoleId( roleId );
  }
  else qWarning() << query.lastError().text();

  return result;
}

bool UserDAO::updatePassword( const QString& username, const QString& password ) const
{
  loadUsers();

  QCryptographicHash hash{ QCryptographicHash::Sha3_512 };
  hash.addData( password.toLocal8Bit() );
  const auto passwd = hash.result().toHex();

  const auto& user = retrieveByUsername( username );
  const auto uptr = User::from( user );

  auto query = CPreparedSqlQueryThreadForDB(
   "update users set password = :password where user_id = :userId", DATABASE_NAME );
  query.bindValue( ":password", passwd );
  query.bindValue( ":userId", uptr->getId() );
  const auto result = query.exec();

  if ( result )
  {
    auto iter = users.find( uptr->getId() );
    iter->second->setPassword( passwd );
    qDebug() << "Updated plain text password for user: " << uptr->getId();
  }
  else qWarning() << query.lastError().text();

  return result;
}
