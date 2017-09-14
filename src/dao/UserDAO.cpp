#include "UserDAO.h"
#include "RoleDAO.h"
#include "constants.h"

#include <mutex>

#include <QtCore/QCryptographicHash>
#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>

namespace crrc
{
  namespace dao
  {
    struct User
    {
      QVariant id;
      QVariant username;
      QVariant password;
      QVariant email;
      QVariant firstName;
      QVariant lastName;
      QVariant middleName;
      QVariant roleId;
    };

    static QMap<uint32_t, User> users;
    static std::atomic_bool usersLoaded{ false };
    static std::mutex userMutex;

    QVariantHash transform( const User& user )
    {
      QVariantHash record;
      record.insert( "user_id", user.id );
      record.insert( "username", user.username );
      record.insert( "password", user.password );

      if ( user.email.isValid() ) record.insert( "email", user.email );
      if ( user.firstName.isValid() ) record.insert( "first_name", user.email );
      if ( user.lastName.isValid() ) record.insert( "last_name", user.email );
      if ( user.middleName.isValid() ) record.insert( "middle_name", user.email );

      if ( user.roleId.isValid() )
      {
        const auto role = RoleDAO().retrieve( user.roleId.toString() );
        if ( !role.isEmpty() ) record.insert( "role", role );
      }

      return record;
    }

    QVariantList fromUsers()
    {
      QVariantList list;
      foreach ( User user, users )
      {
        list.append( transform( user ) );
      }

      return list;
    }

    User createUser( QSqlQuery& query )
    {
      User user;
      user.id = query.value( 0 ).toUInt();
      user.username = query.value( 1 );
      user.password = query.value( 2 );
      user.email = query.value( 3 );
      user.firstName = query.value( 4 );
      user.lastName = query.value( 5 );
      user.middleName = query.value( 6 );
      user.roleId = query.value( 7 ).toUInt();
      return user;
    }

    void loadUsers()
    {
      if ( usersLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ userMutex };
      if ( !users.isEmpty() )
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
          const auto user = createUser( query );
          auto id = user.id.toUInt();
          users.insert( id, std::move( user ) );
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
        const auto r = RoleDAO().retrieve( role );
        query.bindValue( ":role", r.value( "role_id" ).toUInt() );
      }
      else query.bindValue( ":role", QVariant( QVariant::UInt ) );
    }

    User userFromContext( Cutelyst::Context* context )
    {
      User user;
      user.username = context->request()->param( "username" );
      user.password = context->request()->param( "password" );
      user.email = context->request()->param( "email" );
      user.firstName = context->request()->param( "firstName" );
      user.lastName = context->request()->param( "lastName" );
      user.middleName = context->request()->param( "middleName" );

      const auto role = context->request()->param( "role" );
      if ( !role.isEmpty() && role != "-1" ) user.roleId = role.toUInt();
      return user;
    }
  }
}

using crrc::dao::UserDAO;

QVariantList UserDAO::retrieveAll() const
{
  loadUsers();
  return fromUsers();
}

QVariantHash UserDAO::retrieve( const QString& id ) const
{
  loadUsers();
  const auto cid = id.toUInt();
  const auto iter = users.constFind( cid );

  return ( iter != users.end() ) ? 
    transform( iter.value() ) : QVariantHash();
}

QVariantHash UserDAO::retrieveByUsername( const QString& username ) const
{
  loadUsers();
  for ( const auto user : users.values() )
  {
    if ( user.username == username ) return transform( user );
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
  else
  {
    const auto id = query.lastInsertId().toUInt();
    auto user = userFromContext( context );
    user.id = id;
    std::lock_guard<std::mutex> lock{ userMutex };
    users[id] = std::move( user );
    return id;
  }
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
    auto user = userFromContext( context );
    user.id = id.toUInt();
    std::lock_guard<std::mutex> lock{ userMutex };
    users[id.toUInt()] = std::move( user );
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
      auto user = retrieve( query.value( 0 ).toString() );
      list.append( user );
    }
  }
  else
  {
    qWarning() << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
  }

  return list;
}

QString UserDAO::remove( uint32_t id ) const
{
  loadUsers();
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from users where user_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() )
  {
    std::lock_guard<std::mutex> lock{ userMutex };
    users.remove( id );
    return "User deleted.";
  }
  else return query.lastError().text();
}

bool UserDAO::isUsernameAvailable( const QString& username ) const
{
  loadUsers();

  for ( const auto user : users )
  {
    if ( user.username == username ) return false;
  }

  return true;
}

bool UserDAO::updateRole( uint32_t userId, uint32_t roleId ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
   "update users set role_id = :roleId where user_id = :userId", DATABASE_NAME );
  query.bindValue( ":roleId", roleId );
  query.bindValue( ":userId", userId );
  const auto result = query.exec();

  if ( result )
  {
    const auto iter = users.find( userId );
    auto user = iter.value();
    user.roleId = roleId;
    std::lock_guard<std::mutex> lock{ userMutex };
    users[userId] = std::move( user );
  }
  else qWarning() << query.lastError().text();

  return result;
}

bool UserDAO::updatePassword( const QString& username, const QString& password ) const
{
  QCryptographicHash hash{ QCryptographicHash::Sha3_512 };
  hash.addData( password.toLocal8Bit() );
  const auto passwd = hash.result().toHex();
  const auto user = retrieveByUsername( username );
  const auto userId = user.value( "user_id" ).toUInt();

  auto query = CPreparedSqlQueryThreadForDB(
   "update users set password = :password where user_id = :userId", DATABASE_NAME );
  query.bindValue( ":password", passwd );
  query.bindValue( ":userId", userId );
  const auto result = query.exec();

  if ( result )
  {
    const auto iter = users.find( userId );
    auto u = iter.value();
    u.password = passwd;
    std::lock_guard<std::mutex> lock{ userMutex };
    users[userId] = std::move( u );
    qDebug() << "Updated plain text password for user: " << userId;
  }
  else qWarning() << query.lastError().text();

  return result;
}
