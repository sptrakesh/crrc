#include "InstitutionDesignationDAO.h"
#include "constants.h"
#include "DesignationDAO.h"

#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>

namespace crrc
{
  namespace dao
  {
    struct InstitutionDesignation
    {
      QVariant institutionId;
      QVariant designationId;
      QVariant expiration;
    };

    QVariantHash transform( const InstitutionDesignation& id )
    {
      QVariantHash record;
      record.insert( "institution_id", id.institutionId );
      record.insert( "designation", DesignationDAO().retrieve( id.designationId.toUInt() ) );
      record.insert( "expiration", id.expiration.toInt() );
      return record;
    }

    InstitutionDesignation createInstitutionDesignation( QSqlQuery& query )
    {
      InstitutionDesignation id;
      id.institutionId = query.value( 0 ).toUInt();
      id.designationId = query.value( 1 ).toUInt();
      id.expiration = query.value( 2 );
      return id;
    }

    void bindInstitutionDesignation( Cutelyst::Context* c, QSqlQuery& query )
    {
      query.bindValue( ":iid", c->request()->param( "institution_id" ).toUInt() );
      query.bindValue( ":did", c->request()->param( "designation_id" ).toUInt() );
      query.bindValue( ":expiration", c->request()->param( "expiration" ) );
    }

    InstitutionDesignation institutionDesigntationFromContext( Cutelyst::Context* context )
    {
      InstitutionDesignation id;
      id.institutionId = context->request()->param( "institution_id" );
      id.designationId = context->request()->param( "designation_id" );
      id.expiration = context->request()->param( "expiration" );
      return id;
    }
  }
}

using crrc::dao::InstitutionDesignationDAO;

QVariantList InstitutionDesignationDAO::retrieve( uint32_t institution_id ) const
{
  QVariantList list;

  auto query = CPreparedSqlQueryThreadForDB( 
    "select institution_id, designation_id, expiration from institution_designations where institution_id = :iid",
    crrc::DATABASE_NAME );
  query.bindValue( ":iid", institution_id );
  if ( query.exec() )
  {
    while ( query.next() )
    {
      list << transform( createInstitutionDesignation( query ) );
    }
  }

  return list;
}

QString InstitutionDesignationDAO::save( Cutelyst::Context* context ) const
{
  const auto& result = update( context );
  if ( result == "0" ) return insert( context );
  return result;
}

QString InstitutionDesignationDAO::remove( Cutelyst::Context* context ) const
{
  auto query = CPreparedSqlQueryThreadForDB( 
    "delete from institution_designations where institution_id = :iid and designation_id = :did",
    crrc::DATABASE_NAME );
  query.bindValue( ":iid", context->request()->param( "institution_id" ).toUInt() );
  query.bindValue( ":did", context->request()->param( "designation_id" ).toUInt() );
  return query.exec() ? QString() : query.lastError().text();
}

QString InstitutionDesignationDAO::update( Cutelyst::Context* context ) const
{
  auto query = CPreparedSqlQueryThreadForDB( 
    "update institution_designations set expiration = :expiration where institution_id = :iid and designation_id = :did",
    crrc::DATABASE_NAME );
  bindInstitutionDesignation( context, query );
  if ( query.exec() )
  {
    qDebug() << "Update statement succeeded.  Num rows: " << query.numRowsAffected();
    return ( query.numRowsAffected() > 0 ) ? QString() : QString( "0" );
  }

  return query.lastError().text();
}

QString InstitutionDesignationDAO::insert( Cutelyst::Context* context ) const
{
  auto query = CPreparedSqlQueryThreadForDB( 
    "insert into institution_designations (institution_id, designation_id, expiration) values (:iid, :did, :expiration)",
    crrc::DATABASE_NAME );
  bindInstitutionDesignation( context, query );
  return query.exec() ? QString() : query.lastError().text();
}
