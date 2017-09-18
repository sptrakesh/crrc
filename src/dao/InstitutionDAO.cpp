#include "InstitutionDAO.h"
#include "LogoDAO.h"
#include "InstitutionDesignationDAO.h"
#include "constants.h"
#include <mutex>
#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>
#include "ContactDAO.h"


namespace crrc
{
  namespace dao
  {
    struct Institution
    {
      QVariant id;
      QVariant name;
      QVariant address;
      QVariant city;
      QVariant state;
      QVariant postalCode;
      QVariant country;
      QVariant website;
      QVariant logoId;
    };

    static QMap<uint32_t, Institution> institutions;
    static std::atomic_bool institutionsLoaded{ false };
    static std::mutex institutionMutex;

    QVariantHash transform( const Institution& institution, const InstitutionDAO::Mode& mode )
    {
      QVariantHash record;
      record.insert( "institution_id", institution.id );
      record.insert( "name", institution.name );
      record.insert( "city", institution.city );

      if ( InstitutionDAO::Mode::Full == mode )
      {
        record.insert( "address", institution.address );
        record.insert( "state", institution.state );
        record.insert( "postal_code", institution.postalCode );
        record.insert( "country", institution.country );
        record.insert( "website", institution.website );

        if ( !institution.logoId.isNull() )
        {
          auto const& logo = LogoDAO().retrieve( institution.logoId.toString() );
          record.insert( "logo", logo );
        }
      }

      return record;
    }

    QVariantList fromInstitutions( const InstitutionDAO::Mode& mode )
    {
      QVariantList list;
      foreach ( Institution institution, institutions )
      {
        list.append( transform( institution, mode ) );
      }

      return list;
    }

    Institution createInstitution( QSqlQuery& query )
    {
      Institution institution;
      institution.id = query.value( 0 ).toUInt();
      institution.name = query.value( 1 );
      institution.address = query.value( 2 );
      institution.city = query.value( 3 );
      institution.state = query.value( 4 );
      institution.postalCode = query.value( 5 );
      institution.country = query.value( 6 );
      institution.website = query.value( 7 );
      institution.logoId = query.value( 8 );

      return institution;
    }

    void loadInstitutions()
    {
      if ( institutionsLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ institutionMutex };
      if ( !institutions.isEmpty() )
      {
        institutionsLoaded = true;
        return;
      }

      auto query = CPreparedSqlQueryThreadForDB(
        "select institution_id, name, address, city, state, postal_code, country, website, logo_id from institutions order by name", DATABASE_NAME );

      if ( query.exec() )
      {
        while ( query.next() )
        {
          const auto institution = createInstitution( query );
          auto id = institution.id.toUInt();
          institutions.insert( id, std::move( institution ) );
        }

        institutionsLoaded = true;
      }
    }

    void bindInstitution( Cutelyst::Context* c, QSqlQuery& query )
    {
      query.bindValue( ":name", c->request()->param( "name" ) );
      query.bindValue( ":address", c->request()->param( "address" ) );
      query.bindValue( ":city", c->request()->param( "city" ) );
      query.bindValue( ":state", c->request()->param( "state" ) );
      query.bindValue( ":postalCode", c->request()->param( "postalCode" ) );
      auto country = c->request()->param( "country", "" );
      if ( country.isEmpty() ) country = "USA";
      query.bindValue( ":country", country );
      query.bindValue( ":website", c->request()->param( "website" ) );
      query.bindValue( ":logo", c->request()->param( "logoId" ) );
    }

    Institution institutionFromContext( Cutelyst::Context* context, uint32_t id )
    {
      Institution institution;
      institution.id = id;
      institution.name = context->request()->param( "name" );
      institution.address = context->request()->param( "address" );
      institution.city = context->request()->param( "city" );
      institution.state = context->request()->param( "state" );
      institution.postalCode = context->request()->param( "postalCode" );
      institution.country = context->request()->param( "country" );
      institution.website = context->request()->param( "website" );
      institution.logoId = context->request()->param( "logoId" );

      return institution;
    }
  }
}

using crrc::dao::InstitutionDAO;

QVariantList InstitutionDAO::retrieveAll( const Mode& mode ) const
{
  loadInstitutions();
  return fromInstitutions( mode );
}

QVariantHash InstitutionDAO::retrieve( const QString& id, const Mode& mode ) const
{
  loadInstitutions();
  uint32_t cid = id.toUInt();
  const auto iter = institutions.constFind( cid );

  return ( iter != institutions.end() ) ? 
    transform( iter.value(), mode ) : QVariantHash();
}

uint32_t InstitutionDAO::insert( Cutelyst::Context* context ) const
{
  loadInstitutions();
  QSqlQuery query = CPreparedSqlQueryThreadForDB(
    "insert into institutions (name, address, city, state, postal_code, country, website, logo_id) values (:name, :address, :city, :state, :postalCode, :country, :website, :logo)",
    crrc::DATABASE_NAME );
  bindInstitution( context, query );

  if ( !query.exec() )
  {
    context->stash()["error_msg"] = query.lastError().text();
    return 0;
  }
  else
  {
    const auto id = query.lastInsertId().toUInt();
    const auto institution = institutionFromContext( context, id );
    std::lock_guard<std::mutex> lock{ institutionMutex };
    institutions[id] = std::move( institution );
    return id;
  }
}

void InstitutionDAO::update( Cutelyst::Context* context ) const
{
  loadInstitutions();
  auto id = context->request()->param( "id" );
  auto query = CPreparedSqlQueryThreadForDB(
    "update institutions set name=:name, address=:address, city=:city, state=:state, postal_code=:postalCode, country=:country, website=:website, logo_id=:logo where institution_id=:id",
    crrc::DATABASE_NAME );
  bindInstitution( context, query );
  query.bindValue( ":id", id.toInt() );

  if ( query.exec() )
  {
    const auto iid = id.toUInt();
    const auto institution = institutionFromContext( context, iid );
    std::lock_guard<std::mutex> lock{ institutionMutex };
    institutions[iid] = std::move( institution );
  }
  else context->stash()["error_msg"] = query.lastError().text();
}

QVariantList InstitutionDAO::search( Cutelyst::Context* context, const Mode& mode ) const
{
  loadInstitutions();
  const auto text = context->request()->param( "text", "" );
  const QString clause = "%" % text % "%";

  auto query = CPreparedSqlQueryThreadForDB(
   "select institution_id from institutions where name like :text order by name",
    DATABASE_NAME );

  query.bindValue( ":text", clause );
  QVariantList list;

  if ( query.exec() )
  {
    while ( query.next() )
    {
      auto institution = retrieve( query.value( 0 ).toString(), mode );
      list.append( institution );
    }
  }
  else
  {
    qWarning() << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
  }

  return list;
}

QString InstitutionDAO::remove( uint32_t id ) const
{
  loadInstitutions();
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from institutions where institution_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() )
  {
    ContactDAO().removeInstitution( id );
    InstitutionDesignationDAO().retrieve( id );

    const auto& institution = institutions.constFind( id );
    if ( institution != institutions.constEnd() )
    {
      LogoDAO().remove( institution.value().logoId.toUInt() );
    }

    std::lock_guard<std::mutex> lock{ institutionMutex };
    institutions.remove( id );
    return "Institution deleted.";
  }
  else return query.lastError().text();
}

bool InstitutionDAO::isUnique( const QString& name, const QString& city ) const
{
  loadInstitutions();
  for ( const auto& institution : institutions )
  {
    if ( institution.name.toString() == name &&
      institution.city.toString() == city ) return false;
  }

  return true;
}
