#include "InstitutionAgreement.h"
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
