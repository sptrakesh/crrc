#include "InstitutionAgreementDAO.h"
#include "ProgramDAO.h"
#include "DegreeDAO.h"
#include "constants.h"

#include <mutex>
#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>


namespace crrc
{
  namespace dao
  {
    struct InstitutionAgreement
    {
      QVariant agreementId;
      QVariant transferProgram;
      QVariant transfereeProgram;
    };

    QVariantHash createProgramFromId( const QVariant& p )
    {
      QVariantHash transfer;
      ProgramDAO pdao;
      auto program = pdao.retrieve( p.toString() );
      if ( ! program.isEmpty() )
      {
        transfer.insert( "program_id", program.value( "program_id" ).toUInt() );
        transfer.insert( "title", program.value( "title" ) );
        transfer.insert( "credits", program.value( "credits" ) );

        QVariantHash institution;
        const auto& inst = program.value( "institution" ).toHash();
        institution.insert( "institution_id", inst.value( "institution_id" ).toUInt() );
        institution.insert( "name", inst.value( "name" ) );

        transfer.insert( "institution", institution );

        QVariantHash degree;
        const auto& deg = program.value( "degree" ).toHash();
        degree.insert( "degree_id", deg.value( "degree_id" ).toUInt() );
        degree.insert( "title", deg.value( "title" ) );

        transfer.insert( "degree", degree );
      }

      return transfer;
    }

    QVariantHash transform( const InstitutionAgreement& institutionAgreement )
    {
      QVariantHash record;
      record.insert( "agreement_id", institutionAgreement.agreementId );

      record.insert( "transfer_program", 
        createProgramFromId( institutionAgreement.transferProgram ) );
      record.insert( "transferee_program", 
        createProgramFromId( institutionAgreement.transfereeProgram ) );

      return record;
    }

    InstitutionAgreement createInstitutionAgreement( QSqlQuery& query )
    {
      InstitutionAgreement institutionAgreement;
      institutionAgreement.agreementId = query.value( 0 ).toUInt();
      institutionAgreement.transferProgram = query.value( 1 ).toUInt();
      institutionAgreement.transfereeProgram = query.value( 2 ).toUInt();

      return institutionAgreement;
    }

    void bindInstitutionAgreement( Cutelyst::Context* c, QSqlQuery& query )
    {
      query.bindValue( ":id", c->request()->param( "agreement_id" ) );
      query.bindValue( ":tp1", c->request()->param( "transfer_program_id" ) );
      query.bindValue( ":tp2", c->request()->param( "transferee_program_id" ) );
    }
  }
}

using crrc::dao::InstitutionAgreementDAO;

QVariantList InstitutionAgreementDAO::retrieve( const QString& agreementId ) const
{
  return agreementId.contains( "_" ) ? 
    retrieveByKey( agreementId.split( "_" ) ) : 
    retrieveByAgreement( agreementId );
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
    return 0;
  }
  else
  {
    return query.lastInsertId().toUInt();
  }
}

void InstitutionAgreementDAO::remove( Cutelyst::Context* context ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from institution_agreements where agreement_id = :id and transfer_program_id = :tp1 and transferee_program_id = :tp2",
    DATABASE_NAME );
  bindInstitutionAgreement( context, query );

  if ( ! query.exec() ) context->stash()["error_msg"] = query.lastError().text();
}

QVariantList InstitutionAgreementDAO::retrieveByAgreement( const QString& agreementId ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
    "select * from institution_agreements where agreement_id = :id",
    crrc::DATABASE_NAME );
  query.bindValue( ":id", agreementId );
  return toList( query );
}

QVariantList InstitutionAgreementDAO::retrieveByKey( const QStringList& ids ) const
{
  QVariantList list;
  auto query = CPreparedSqlQueryThreadForDB(
    "select * from institution_agreements where agreement_id = :id and transfer_program_id = :tp1 and transferee_program_id = :tp2",
    crrc::DATABASE_NAME );
  query.bindValue( ":id", ids.at( 0 ) );
  query.bindValue( ":tp1", ids.at( 1 ) );
  query.bindValue( ":tp2", ids.at( 2 ) );
  return toList( query );
}

QVariantList InstitutionAgreementDAO::toList( QSqlQuery& query ) const
{
  QVariantList list;

  if ( query.exec() )
  {
    while ( query.next() )
    {
      const auto& ia = createInstitutionAgreement( query );
      list.append( transform( ia ) );
    }
  }

  return list;
}