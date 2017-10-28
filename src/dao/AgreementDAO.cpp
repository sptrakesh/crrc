#include "AgreementDAO.h"
#include "InstitutionDAO.h"
#include "constants.h"
#include "functions.h"
#include "model/Agreement.h"

#include <mutex>
#include <unordered_map>

#include <QtCore/QLoggingCategory>
#include <QtSql/QtSql>
#include <Cutelyst/Upload>
#include <Cutelyst/Plugins/Utils/sql.h>

using crrc::model::Agreement;

Q_LOGGING_CATEGORY( AGREEMENT_DAO, "crrc.model.AgreementDAO" )

namespace crrc
{
  namespace dao
  {
    static std::unordered_map<uint32_t, Agreement::Ptr> agreements;
    static std::atomic_bool agreementsLoaded{ false };
    static std::mutex agreementMutex;

    QVariantList fromAgreements()
    {
      QVariantList list;
      for ( const auto& iter : agreements ) list << asVariant( iter.second.get() );
      return list;
    }

    void loadAgreements()
    {
      if ( agreementsLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ agreementMutex };
      if ( !agreements.empty() )
      {
        agreementsLoaded = true;
        return;
      }

      auto query = CPreparedSqlQueryThreadForDB(
        R"(
select agreement_id, filename, mimetype, filesize, checksum, updated,
  transfer_institution_id, transferee_institution_id
from agreements order by filename
)",
        DATABASE_NAME );

      if ( query.exec() )
      {
        while ( query.next() )
        {
          auto agreement = Agreement::create( query );
          agreements[agreement->getId()] = std::move( agreement );
        }

        agreementsLoaded = true;
      }
    }

    QByteArray bindAgreement( Cutelyst::Context* c, QSqlQuery& query )
    {
      auto upload = c->request()->upload( "file" );
      if ( !upload ) return QByteArray();

      auto bytes = upload->readAll();
      const QFileInfo file{ upload->filename() };
      qDebug( AGREEMENT_DAO ) << "File: " << upload;
      query.bindValue( ":filename", file.fileName() );
      query.bindValue( ":mimetype", upload->contentType() );
      query.bindValue( ":filesize", upload->size() );
      query.bindValue( ":document", bytes );
      query.bindValue( ":checksum", checksum( bytes ) );
      query.bindValue( ":updated", httpDate( file.lastModified() ) );

      return bytes;
    }
  }
}

using crrc::dao::AgreementDAO;

QVariantList AgreementDAO::retrieveAll() const
{
  loadAgreements();
  return fromAgreements();
}

QVariantList AgreementDAO::retrieveByInstitution( uint32_t id ) const
{
  loadAgreements();
  QVariantList list;
  if ( !id ) return list;

  for ( const auto& iter : agreements )
  {
    if ( id == iter.second->getTransferInstitutionId() || 
      id == iter.second->getTransfereeInstitutionId() )
    {
      list << asVariant( iter.second.get() );
    }
  }

  return list;
}

QVariant AgreementDAO::retrieve( const uint32_t id ) const
{
  loadAgreements();
  const auto iter = agreements.find( id );

  return ( iter != agreements.end() ) ? 
    asVariant( iter->second.get() ) : QVariant();
}

QByteArray AgreementDAO::contents( Cutelyst::Context* context, const uint32_t id ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
    "select document from agreements where agreement_id = :id",
    crrc::DATABASE_NAME );
  query.bindValue( ":id", id );

  if ( query.exec() )
  {
    query.first();
    return query.value( 0 ).toByteArray();
  }

  context->stash()["error_msg"] = query.lastError().text();
  qWarning( AGREEMENT_DAO ) << query.lastError().text();
  return QByteArray();
}


uint32_t AgreementDAO::insert( Cutelyst::Context* context ) const
{
  qDebug( AGREEMENT_DAO ) << "Inserting new agreement document";
  auto query = CPreparedSqlQueryThreadForDB(
    R"(
insert into agreements
(filename, mimetype, filesize, document, checksum, updated)
values
(:filename, :mimetype, :filesize, :document, :checksum, :updated)
)",
    crrc::DATABASE_NAME );
  auto bytes = bindAgreement( context, query );

  if ( query.exec() && query.numRowsAffected() )
  {
    auto id = query.lastInsertId().toUInt();
    auto agreement = Agreement::create( context, bytes );
    agreement->setId( id );
    std::lock_guard<std::mutex> lock{ agreementMutex };
    agreements[id] = std::move( agreement );
    return id;
  }

  context->stash()["error_msg"] = query.lastError().text();
  qWarning( AGREEMENT_DAO ) << query.lastError().text();
  return 0;
}

uint32_t AgreementDAO::update( Cutelyst::Context* context ) const
{
  auto id = context->request()->param( "id" );
  if ( id.isEmpty() )
  {
    const auto& obj = context->stash( "object" );
    if ( !obj.isNull() ) id = Agreement::from( obj )->getId();
  }

  auto query = CPreparedSqlQueryThreadForDB( 
    R"(
update agreements set filename=:filename, mimetype=:mimetype, filesize=:filesize,
  document=:document, checksum=:checksum, updated=:updated
where agreement_id=:id
)",
    crrc::DATABASE_NAME );
  auto bytes = bindAgreement( context, query );
  query.bindValue( ":id", id.toInt() );

  if ( query.exec() && query.numRowsAffected() )
  {
    auto agreement = Agreement::create( context, bytes );
    const auto aid = id.toUInt();
    agreement->setId( aid );

    std::lock_guard<std::mutex> lock{ agreementMutex };
    agreements[aid] = std::move( agreement );
    return query.numRowsAffected();
  }

  qWarning( AGREEMENT_DAO ) << query.lastError().text();
  context->stash()["error_msg"] = query.lastError().text();
  return 0;
}

QVariantList AgreementDAO::search( Cutelyst::Context* context ) const
{
  const auto text = context->request()->param( "text", "" );
  const QString clause = "%" % text % "%";

  auto query = CPreparedSqlQueryThreadForDB(
   R"(
select agreement_id from agreements
where filename like :text
order by filename
)",
    DATABASE_NAME );

  query.bindValue( ":text", clause );
  QVariantList list;

  if ( query.exec() )
  {
    while ( query.next() ) list << retrieve( query.value( 0 ).toUInt() );
  }
  else
  {
    qWarning( AGREEMENT_DAO ) << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
  }

  return list;
}

uint32_t AgreementDAO::remove( uint32_t id ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from agreements where agreement_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() )
  {
    const auto count = query.numRowsAffected();
    std::lock_guard<std::mutex> lock{ agreementMutex };
    agreements.erase( id );
    return count;
  }

  qWarning( AGREEMENT_DAO ) << query.lastError().text();
  return 0;
}

uint32_t AgreementDAO::saveProgram( Cutelyst::Context* context ) const
{
  const auto id = context->request()->param( "id" ).toUInt();

  auto query = CPreparedSqlQueryThreadForDB(
    R"(
update agreements set transfer_institution_id = :i,
  transferee_institution_id = :ei
where agreement_id = :id
)",
    DATABASE_NAME );
  query.bindValue( ":i", context->request()->param( "transfer_institution_id" ).toUInt() );
  query.bindValue( ":ei", context->request()->param( "transferee_institution_id" ).toUInt() );
  query.bindValue( ":id", context->request()->param( "id" ).toUInt() );

  if ( query.exec() )
  {
    if ( query.numRowsAffected() > 0 )
    {
      auto iter = agreements.find( id );
      if ( iter != agreements.end() ) iter->second->updatePrograms( context );
      return query.numRowsAffected();
    }
  }

  qWarning( AGREEMENT_DAO ) << query.lastError().text();
  context->stash()["error_msg"] = query.lastError().text();
  return 0;
}
