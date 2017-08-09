#include "InstitutionDAO.h"
#include "ContactDAO.h"
#include "constants.h"
#include <mutex>
#include <QtCore/QStringBuilder>
#include <QtSql/QtSql>
#include <Cutelyst/Plugins/Utils/sql.h>


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
      QVariant contactId;
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

        if ( ! institution.contactId.isNull() )
        {
          auto c = ContactDAO().retrieve(
            institution.contactId.toString(), ContactDAO::Mode::Partial );
          if ( ! c.isEmpty() )
          {
            QVariantHash contact;
            contact.insert( "contact_id", institution.contactId.toUInt() );
            contact.insert( "name", c.value( "name" ) );
            record.insert( "contact", contact );
          }
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
      const auto contactId = query.value( 8 );
      institution.contactId = contactId.isNull() ? contactId : contactId.toUInt();
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
        "select * from institutions order by name", DATABASE_NAME );

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

    void loadInstitution( uint32_t id )
    {
      auto query = CPreparedSqlQueryThreadForDB(
        "select * from institutions institution_id = :id",
        DATABASE_NAME );
      query.bindValue( ":id", id );
      if ( query.exec() )
      {
        query.next();
        const auto institution = createInstitution( query );
        std::lock_guard<std::mutex> lock{ institutionMutex };
        institutions.insert( id, std::move( institution ) );
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

      auto contact = c->request()->param( "contact", "" );
      if ( contact.isEmpty() || contact == "0" ) query.bindValue( ":contact", QVariant() );
      else query.bindValue( ":contact", c->request()->param( "contact" ).toInt() );
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

      const auto contactId = context->request()->param( "contact", "0" );
      institution.contactId = ( contactId != "0" ) ?  contactId.toUInt() : QVariant();

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
    "insert into institutions (name, address, city, state, postal_code, country, website, contact_id) values (:name, :address, :city, :state, :postalCode, :country, :website, :contact)",
    crrc::DATABASE_NAME );
  bindInstitution( context, query );

  if ( !query.exec() )
  {
    context->stash()["error_msg"] = query.lastError().text();
    return 0;
  }
  else
  {
    auto id = query.lastInsertId().toUInt();
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
    "update institutions set name=:name, address=:address, city=:city, state=:state, postal_code=:postalCode, country=:country, website=:website, contact_id=:contact where institution_id=:id",
    crrc::DATABASE_NAME );
  bindInstitution( context, query );
  query.bindValue( ":id", id.toInt() );

  if ( query.exec() )
  {
    auto iid = id.toUInt();
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
    std::lock_guard<std::mutex> lock{ institutionMutex };
    institutions.remove( id );
    return "Institution deleted.";
  }
  else return query.lastError().text();
}
