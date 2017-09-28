#include "degree.h"

using crrc::model::Degree;

Degree::Ptr Degree::create( QSqlQuery& query )
{
  auto degree = std::make_unique<Degree>();
  degree->id = query.value( 0 ).toUInt();
  degree->title = query.value( 1 ).toString();
  degree->duration = query.value( 2 ).toString();
  return degree;
}

Degree::Ptr Degree::create( Cutelyst::Context* context )
{
  auto degree = std::make_unique<Degree>();

  const auto& id = context->request()->param( "id" );
  if ( !id.isEmpty() ) degree->id = id.toUInt();

  degree->title = context->request()->param( "title" );
  degree->duration = context->request()->param( "duration" );

  return degree;
}
