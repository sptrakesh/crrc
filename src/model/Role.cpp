#include "Role.h"

using crrc::model::Role;

Role::Ptr Role::create( QSqlQuery& query )
{
  auto ptr = std::make_unique<Role>();
  ptr->id = query.value( 0 ).toUInt();
  ptr->role = query.value( 1 ).toString();
  return ptr;
}

Role::Ptr Role::create( Cutelyst::Context* context )
{
  auto ptr = std::make_unique<Role>();

  const auto& id = context->request()->param( "id" );
  if ( !id.isEmpty() ) ptr->id = id.toUInt();

  ptr->role = context->request()->param( "role" );
  return ptr;
}

Role::operator QString() const
{
  return QString( "Role - id: (%1), role: (%2)" ).arg( id ).arg( role );
}
