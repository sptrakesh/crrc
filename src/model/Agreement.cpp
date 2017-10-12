#include "Agreement.h"
#include "dao/InstitutionDAO.h"
#include "Institution.h"

using crrc::model::Agreement;

Agreement::Ptr Agreement::create( QSqlQuery& query )
{
  auto ptr = std::make_unique<Agreement>();
  ptr->populate( query );
  ptr->transferInstitutionId = query.value( 6 ).toUInt();
  ptr->transfereeInstitutionId = query.value( 7 ).toUInt();
  return ptr;
}

Agreement::Ptr Agreement::create( Cutelyst::Context* context, const QByteArray& bytes )
{
  auto ptr = std::make_unique<Agreement>();
  ptr->populate( context, bytes );
  return ptr;
}

QVariant Agreement::getTransferInstitution() const
{
  return transferInstitutionId ?
    dao::InstitutionDAO().retrieve( transferInstitutionId ) : QVariant();
}

QVariant Agreement::getTransfereeInstitution() const
{
  return transfereeInstitutionId ?
    dao::InstitutionDAO().retrieve( transfereeInstitutionId ) : QVariant();
}

Agreement& Agreement::updatePrograms( Cutelyst::Context* context )
{
  transferInstitutionId = context->request()->param( "transfer_institution_id" ).toUInt();
  transfereeInstitutionId = context->request()->param( "transferee_institution_id" ).toUInt();
  return *this;
}

QJsonObject crrc::model::toJson( const Agreement& agreement )
{
  const auto ptr = dynamic_cast<const BlobItem*>( &agreement );
  QJsonObject json = toJson( *ptr );
  json.insert( "transferInstitution", toJson( *( Institution::from( agreement.getTransferInstitution() ) ), true ) );
  json.insert( "transfereeInstitution", toJson( *( Institution::from( agreement.getTransfereeInstitution() ) ), true ) );
  return json;
}
