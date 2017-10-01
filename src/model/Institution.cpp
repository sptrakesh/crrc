#include "Institution.h"
#include "dao/LogoDAO.h"

using crrc::model::Institution;

Institution::Ptr Institution::create( QSqlQuery& query )
{
  auto ptr = std::make_unique<Institution>();
  ptr->id = query.value( 0 ).toUInt();
  ptr->name = query.value( 1 ).toString();
  ptr->address = query.value( 2 ).toString();
  ptr->city = query.value( 3 ).toString();
  ptr->state = query.value( 4 ).toString();
  ptr->postalCode = query.value( 5 ).toString();
  ptr->country = query.value( 6 ).toString();
  ptr->website = query.value( 7 ).toString();
  ptr->logoId = query.value( 8 ).toUInt();
  return ptr;
}

Institution::Ptr Institution::create( Cutelyst::Context* context )
{
  auto ptr = std::make_unique<Institution>();

  const auto& id = context->request()->param( "id" );
  if ( !id.isEmpty() ) ptr->id = id.toUInt();

  ptr->name = context->request()->param( "name" );
  ptr->address = context->request()->param( "address" );
  ptr->city = context->request()->param( "city" );
  ptr->state = context->request()->param( "state" );
  ptr->postalCode = context->request()->param( "postalCode" );
  ptr->country = context->request()->param( "country" );
  ptr->website = context->request()->param( "website" );
  ptr->logoId = context->request()->param( "logoId" ).toUInt();

  return ptr;
}

QVariant Institution::getLogo() const
{
  return dao::LogoDAO().retrieve( logoId );
}
