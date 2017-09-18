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
      record.insert( "designation", DesignationDAO().retrieve( id.designationId.toString() ) );
      record.insert( "expiration", id.expiration );
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
      query.bindValue( ":iid", c->request()->param( "institution_id" ) );
      query.bindValue( ":did", c->request()->param( "designation_id" ) );
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
    "select institution_id, designation_id, expiration from institution_designations where institution_id = :id",
    crrc::DATABASE_NAME );
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
  auto update = CPreparedSqlQueryThreadForDB( 
    "update institution_designations set expiration = :expiration where institution_id = :iid and designation_id = :did",
    crrc::DATABASE_NAME );
  bindInstitutionDesignation( context, update );
  if ( update.exec() )
  {
    if ( update.numRowsAffected() > 0 ) return QString();

    auto insert = CPreparedSqlQueryThreadForDB( 
      "insert into institution_designations (institution_id, designation_id, expiration) values (:iid, :did, :expiration)",
      crrc::DATABASE_NAME );
    return insert.exec() ? QString() : insert.lastError().text();
  }
  else return update.lastError().text();
}

QString InstitutionDesignationDAO::remove( Cutelyst::Context* context ) const
{
  auto query = CPreparedSqlQueryThreadForDB( 
    "delete from institution_designations where institution_id = :iid and designation_id = :did",
    crrc::DATABASE_NAME );
  query.bindValue( ":iid", context->request()->param( "institution_id" ) );
  query.bindValue( ":did", context->request()->param( "designation_id" ) );
  return query.exec() ? QString() : query.lastError().text();
}
