#include "InstitutionType.h"

using crrc::model::InstitutionType;

InstitutionType::Ptr InstitutionType::create( QSqlQuery& query )
{
  auto ptr = std::make_unique<InstitutionType>();
  ptr->id = query.value( 0 ).toUInt();
  ptr->name = query.value( 1 ).toString();
  return ptr;
}

QJsonObject crrc::model::toJson( const InstitutionType& instType )
{
  QJsonObject obj;
  obj.insert( "id", static_cast<int>( instType.getId() ) );
  obj.insert( "name", instType.getName() );
  return obj;
}
