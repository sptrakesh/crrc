#include "InstitutionDesignation.h"
#include "Institution.h"
#include "Designation.h"
#include "dao/InstitutionDAO.h"
#include "dao/DesignationDAO.h"

using crrc::model::InstitutionDesignation;

InstitutionDesignation* InstitutionDesignation::create(
  Cutelyst::Context* context, QSqlQuery& query )
{
  const auto ptr = new InstitutionDesignation( context );
  ptr->institutionId = query.value( 0 ).toUInt();
  ptr->designationId = query.value( 1 ).toUInt();
  ptr->expiration = query.value( 2 ).toUInt();
  return ptr;
}

QVariant InstitutionDesignation::getInstitution() const
{
  return dao::InstitutionDAO().retrieve( institutionId );
}

QVariant InstitutionDesignation::getDesignation() const
{
  return dao::DesignationDAO().retrieve( designationId );
}

QJsonObject crrc::model::toJson( const InstitutionDesignation& id )
{
  QJsonObject obj;
  obj.insert( "institution", toJson( *( Institution::from( id.getInstitution() ) ), true ) );
  obj.insert( "designation", toJson( *( Designation::from( id.getDesignation() ) ) ) );
  if ( id.getExpiration() ) obj.insert( "expiration", static_cast<int>( id.getExpiration() ) );
  return obj;
}
