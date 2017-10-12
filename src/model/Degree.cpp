#include "Degree.h"

using crrc::model::Degree;

Degree::Ptr Degree::create( QSqlQuery& query )
{
  auto ptr = std::make_unique<Degree>();
  ptr->id = query.value( 0 ).toUInt();
  ptr->title = query.value( 1 ).toString();
  ptr->duration = query.value( 2 ).toString();
  return ptr;
}

Degree::Ptr Degree::create( Cutelyst::Context* context )
{
  auto ptr = std::make_unique<Degree>();

  const auto& id = context->request()->param( "id" );
  if ( !id.isEmpty() ) ptr->id = id.toUInt();

  ptr->title = context->request()->param( "title" );
  ptr->duration = context->request()->param( "duration" );

  return ptr;
}

Degree::operator QString() const
{
  return QString( "Degree - id: (%1), title: (%2), duration: (%3)" ).
    arg( id ).arg( title ).arg( duration );
}

QJsonObject crrc::model::toJson( const Degree& degree )
{
  QJsonObject obj;
  obj.insert( "id", static_cast<int>( degree.getId() ) );
  obj.insert( "title", degree.getTitle() );
  if ( !degree.getDuration().isEmpty() ) obj.insert( "duration", degree.getDuration() );
  return obj;
}
