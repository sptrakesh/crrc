#include "Designation.h"

using crrc::model::Designation;

Designation::Ptr Designation::create( QSqlQuery& query )
{
  auto ptr = std::make_unique<Designation>();
  ptr->id = query.value( 0 ).toUInt();
  ptr->type = query.value( 1 ).toString();
  ptr->title = query.value( 2 ).toString();
  return ptr;
}

Designation::Ptr Designation::create( Cutelyst::Context* context )
{
  auto ptr = std::make_unique<Designation>();

  const auto& id = context->request()->param( "id" );
  if ( !id.isEmpty() ) ptr->id = id.toUInt();

  ptr->type = context->request()->param( "type" );
  ptr->title = context->request()->param( "title" );

  return ptr;
}

Designation::operator QString() const
{
  return QString( "Designation - id: (%1), type: (%2), title: (%3)" ).
    arg( id ).arg( type ).arg( title );
}

QJsonObject crrc::model::toJson( const Designation& designation )
{
  QJsonObject obj;
  obj.insert( "id", static_cast<int>( designation.getId() ) );
  obj.insert( "type", designation.getType() );
  obj.insert( "title", designation.getTitle() );
  return obj;
}
