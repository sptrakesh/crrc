#include "User.h"
#include "dao/RoleDAO.h"
#include "Role.h"

using crrc::model::User;

User::Ptr User::create( QSqlQuery& query )
{
  auto user = std::make_unique<User>();
  user->id = query.value( 0 ).toUInt();
  user->username = query.value( 1 ).toString();
  user->password = query.value( 2 );
  user->email = query.value( 3 ).toString();
  user->firstName = query.value( 4 ).toString();
  user->lastName = query.value( 5 ).toString();
  user->middleName = query.value( 6 ).toString();
  user->roleId = query.value( 7 ).toUInt();
  return user;
}

User::Ptr User::create( Cutelyst::Context* context )
{
  auto user = std::make_unique<User>();

  const auto& id = context->request()->param( "id" );
  if ( !id.isEmpty() ) user->id = id.toUInt();

  user->username = context->request()->param( "username" );
  user->password = context->request()->param( "password" );
  user->email = context->request()->param( "email" );
  user->firstName = context->request()->param( "firstName" );
  user->lastName = context->request()->param( "lastName" );
  user->middleName = context->request()->param( "middleName" );

  const auto& rid = context->request()->param( "role" );
  if ( !rid.isEmpty() ) user->roleId = rid.toUInt();

  return user;
}

const QVariant User::getRole() const
{
  if ( !roleId ) return QVariant();
  return dao::RoleDAO().retrieve( roleId );
}

User::operator QString() const
{
  return QString( "User - id: (%1), username: (%2), email: (%3), firstName: (%4), lastName: (%5), middleName: (%6), role: (%7)" ).
    arg( id ).arg( username ).arg( email ).arg( firstName ).arg( lastName ).
    arg( middleName ).arg( roleId );
}

QJsonObject crrc::model::toJson( const User& user )
{
  QJsonObject obj;
  obj.insert( "id", static_cast<int>( user.getId() ) );
  obj.insert( "username", user.getUsername() );
  if ( !user.getEmail().isEmpty() ) obj.insert( "email", user.getEmail() );
  if ( !user.getFirstName().isEmpty() ) obj.insert( "firstName", user.getFirstName() );
  if ( !user.getLastName().isEmpty() ) obj.insert( "lastName", user.getLastName() );
  if ( !user.getMiddleName().isEmpty() ) obj.insert( "middleName", user.getMiddleName() );

  if ( user.getRoleId() )
  {
    const auto ptr = Role::from( user.getRole() );
    obj.insert( "role", toJson( *ptr ) );
  }

  return obj;
}
