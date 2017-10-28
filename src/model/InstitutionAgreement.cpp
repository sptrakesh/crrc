#include "InstitutionAgreement.h"
#include "Agreement.h"
#include "Degree.h"
#include "Institution.h"
#include "Program.h"
#include "dao/AgreementDAO.h"
#include "dao/ProgramDAO.h"

using crrc::model::InstitutionAgreement;

InstitutionAgreement* InstitutionAgreement::create( Cutelyst::Context* context, QSqlQuery& query )
{
  const auto ptr = new InstitutionAgreement( context );
  ptr->agreementId = query.value( 0 ).toUInt();
  ptr->transferProgramId = query.value( 1 ).toUInt();
  ptr->transfereeProgramId = query.value( 2 ).toUInt();
  return ptr;
}

QVariant InstitutionAgreement::getAgreement() const
{
  return dao::AgreementDAO().retrieve( agreementId );
}

QVariant InstitutionAgreement::getTransferProgram() const
{
  return dao::ProgramDAO().retrieve( transferProgramId );
}

QVariant InstitutionAgreement::getTransfereeProgram() const
{
  return dao::ProgramDAO().retrieve( transfereeProgramId );
}

QJsonObject crrc::model::toJson( const InstitutionAgreement& ia )
{
  QJsonObject obj;

  const auto aptr = Agreement::from( ia.getAgreement() );
  QJsonObject agreement;
  agreement.insert( "id", static_cast<int>( ia.getAgreementId() ) );
  agreement.insert( "filename", aptr->getFilename() );
  agreement.insert( "mimetype", aptr->getMimetype() );
  obj.insert( "agreement", agreement );

  obj.insert( "transferProgram", toJson( *( Program::from( ia.getTransferProgram() ) ), true ) );
  obj.insert( "transfereeProgram", toJson( *( Program::from( ia.getTransfereeProgram() ) ), true ) );

  return obj;
}
