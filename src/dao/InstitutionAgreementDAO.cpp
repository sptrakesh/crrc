#include "InstitutionAgreementDAO.h"
#include "ProgramDAO.h"
#include "constants.h"
#include "model/InstitutionAgreement.h"

#include <mutex>
#include <QtCore/QLoggingCategory>
#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>

Q_LOGGING_CATEGORY( INSITUTIONAGREEMENTDAO, "crrc.dao.InstitutionAgreementDAO" )

using crrc::model::InstitutionAgreement;

namespace crrc
{
  namespace dao
  {
    void bindInstitutionAgreement( Cutelyst::Context* c, QSqlQuery& query )
    {
      query.bindValue( ":id", c->request()->param( "agreementId" ) );
      query.bindValue( ":tp1", c->request()->param( "transferProgramId" ) );
      query.bindValue( ":tp2", c->request()->param( "transfereeProgramId" ) );
    }
  }
}

using crrc::dao::InstitutionAgreementDAO;

QVariantList InstitutionAgreementDAO::retrieve( Cutelyst::Context* context, 
  const QString& agreementId ) const
{
  return agreementId.contains( "_" ) ? 
    retrieveByKey( context, agreementId.split( "_" ) ) : 
    retrieveByAgreement( context, agreementId.toUInt() );
}

uint32_t InstitutionAgreementDAO::insert( Cutelyst::Context* context ) const
{
  QSqlQuery query = CPreparedSqlQueryThreadForDB(
    "insert into institution_agreements (agreement_id, transfer_program_id, transferee_program_id) values (:id, :tp1, :tp2)",
    crrc::DATABASE_NAME );
  bindInstitutionAgreement( context, query );

  if ( !query.exec() )
  {
    context->stash()["error_msg"] = query.lastError().text();
    qWarning( INSITUTIONAGREEMENTDAO ) << query.lastError().text();
    return 0;
  }

  return query.lastInsertId().toUInt();
}

void InstitutionAgreementDAO::remove( Cutelyst::Context* context ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from institution_agreements where agreement_id = :id and transfer_program_id = :tp1 and transferee_program_id = :tp2",
    DATABASE_NAME );
  bindInstitutionAgreement( context, query );

  if ( ! query.exec() ) context->stash()["error_msg"] = query.lastError().text();
}

QVariantList InstitutionAgreementDAO::retrieveByAgreement( 
  Cutelyst::Context* context, const uint32_t agreementId ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
    "select * from institution_agreements where agreement_id = :id",
    crrc::DATABASE_NAME );
  query.bindValue( ":id", agreementId );
  return toList( context, query );
}

QVariantList InstitutionAgreementDAO::retrieveByKey(
  Cutelyst::Context* context, const QStringList& ids ) const
{
  QVariantList list;
  auto query = CPreparedSqlQueryThreadForDB(
    "select * from institution_agreements where agreement_id = :id and transfer_program_id = :tp1 and transferee_program_id = :tp2",
    crrc::DATABASE_NAME );
  query.bindValue( ":id", ids.at( 0 ) );
  query.bindValue( ":tp1", ids.at( 1 ) );
  query.bindValue( ":tp2", ids.at( 2 ) );
  return toList( context, query );
}

QVariantList InstitutionAgreementDAO::toList( Cutelyst::Context* context, QSqlQuery& query ) const
{
  QVariantList list;

  if ( query.exec() )
  {
    while ( query.next() )
    {
      list << asVariant( InstitutionAgreement::create( context, query ) );
    }
  }

  return list;
}