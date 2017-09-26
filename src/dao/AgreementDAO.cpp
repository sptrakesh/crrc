#include "AgreementDAO.h"
#include "InstitutionDAO.h"
#include "constants.h"
#include "functions.h"

#include <mutex>
#include <QtSql/QtSql>
#include <Cutelyst/Upload>
#include <Cutelyst/Plugins/Utils/sql.h>


namespace crrc
{
  namespace dao
  {
    struct Agreement
    {
      QVariant id;
      QVariant filename;
      QVariant mimetype;
      QVariant filesize;
      QVariant document;
      QVariant checksum;
      QVariant updated;
      QVariant transferInstitutionId;
      QVariant transfereeInstitutionId;
    };

    static QMap<uint32_t, Agreement> agreements;
    static std::atomic_bool agreementsLoaded{ false };
    static std::mutex agreementMutex;

    QVariantHash transform( const Agreement& agreement, const AgreementDAO::Mode& mode )
    {
      QVariantHash record;
      record.insert( "id", agreement.id );
      record.insert( "filename", agreement.filename );

      if ( ! agreement.transferInstitutionId.isNull() )
      {
        const auto im = InstitutionDAO::Mode::Partial;
        const auto idao = InstitutionDAO();
        record.insert( "transferInstitution",
          idao.retrieve( agreement.transferInstitutionId.toString(), im ) );
        record.insert( "transfereeInstitution",
          idao.retrieve( agreement.transfereeInstitutionId.toString(), im ) );
      }

      if ( AgreementDAO::Mode::Full == mode )
      {
        record.insert( "mimetype", agreement.mimetype );
        record.insert( "filesize", agreement.filesize );
        record.insert( "checksum", agreement.checksum );
        record.insert( "updated", agreement.updated );
        record.insert( "document", agreement.document );
      }

      return record;
    }

    QVariantList fromAgreements( const AgreementDAO::Mode& mode )
    {
      QVariantList list;
      foreach ( Agreement agreement, agreements )
      {
        list.append( transform( agreement, mode ) );
      }

      return list;
    }

    Agreement createAgreement( QSqlQuery& query )
    {
      Agreement agreement;
      agreement.id = query.value( 0 ).toUInt();
      agreement.filename = query.value( 1 );
      agreement.mimetype = query.value( 2 );
      agreement.filesize = query.value( 3 );
      agreement.checksum = query.value( 4 );
      agreement.updated = query.value( 5 );
      agreement.transferInstitutionId = query.value( 6 );
      agreement.transfereeInstitutionId = query.value( 7 );
      return agreement;
    }

    void loadAgreements()
    {
      if ( agreementsLoaded.load() ) return;
      std::lock_guard<std::mutex> lock{ agreementMutex };
      if ( !agreements.isEmpty() )
      {
        agreementsLoaded = true;
        return;
      }

      auto query = CPreparedSqlQueryThreadForDB(
        "select agreement_id, filename, mimetype, filesize, checksum, updated, transfer_institution_id, transferee_institution_id from agreements order by filename",
        DATABASE_NAME );

      if ( query.exec() )
      {
        while ( query.next() )
        {
          const auto agreement = createAgreement( query );
          auto id = agreement.id.toUInt();
          agreements.insert( id, std::move( agreement ) );
        }

        agreementsLoaded = true;
      }
    }

    QByteArray bindAgreement( Cutelyst::Context* c, QSqlQuery& query )
    {
      auto upload = c->request()->upload( "document" );
      if ( !upload ) return QByteArray();

      auto bytes = upload->readAll();
      const QFileInfo file{ upload->filename() };
      query.bindValue( ":filename", file.fileName() );
      query.bindValue( ":mimetype", upload->contentType() );
      query.bindValue( ":filesize", upload->size() );
      query.bindValue( ":document", bytes );
      query.bindValue( ":checksum", checksum( bytes ) );
      query.bindValue( ":updated", httpDate( file.lastModified() ) );

      return bytes;
    }

    Agreement agreementFromContext( Cutelyst::Context* context, const QByteArray& bytes )
    {
      Agreement agreement;
      const auto upload = context->request()->upload( "document" );
      const QFileInfo file{ upload->filename() };
      agreement.filename = file.fileName();
      agreement.mimetype = upload->contentType();
      agreement.filesize = upload->size();
      agreement.checksum = checksum( bytes );
      agreement.updated = httpDate( file.lastModified() );
      return agreement;
    }
  }
}

using crrc::dao::AgreementDAO;

QVariantList AgreementDAO::retrieveAll( const Mode& mode ) const
{
  loadAgreements();
  return fromAgreements( mode );
}

QVariantList AgreementDAO::retrieveByInstitution( uint32_t id, const Mode& mode ) const
{
  loadAgreements();
  QVariantList list;
  if ( !id ) return list;

  for ( const auto& agreement : agreements )
  {
    if ( id == agreement.transferInstitutionId.toUInt() || 
      id == agreement.transfereeInstitutionId.toUInt() )
    {
      list << transform( agreement, mode );
    }
  }

  return list;
}

QVariantHash AgreementDAO::retrieve( const QString& id, const Mode& mode ) const
{
  loadAgreements();
  uint32_t cid = id.toUInt();
  const auto iter = agreements.constFind( cid );

  return ( iter != agreements.end() ) ? 
    transform( iter.value(), mode ) : QVariantHash();
}

QByteArray AgreementDAO::contents( Cutelyst::Context* context, const QString& id ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
    "select document from agreements where agreement_id = :id",
    crrc::DATABASE_NAME );
  query.bindValue( ":id", id.toUInt() );

  if ( query.exec() )
  {
    query.first();
    return query.value( 0 ).toByteArray();
  }

  context->stash()["error_msg"] = query.lastError().text();
  return QByteArray();
}


uint32_t AgreementDAO::insert( Cutelyst::Context* context ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
    "insert into agreements (filename, mimetype, filesize, document, checksum, updated) values (:filename, :mimetype, :filesize, :document, :checksum, :updated)",
    crrc::DATABASE_NAME );
  auto bytes = bindAgreement( context, query );

  if ( !query.exec() )
  {
    context->stash()["error_msg"] = query.lastError().text();
    return 0;
  }

  auto id = query.lastInsertId().toUInt();
  auto agreement = agreementFromContext( context, bytes );
  agreement.id = id;
  std::lock_guard<std::mutex> lock{ agreementMutex };
  agreements[id] = std::move( agreement );
  return id;
}

void AgreementDAO::update( Cutelyst::Context* context ) const
{
  auto id = context->request()->param( "id" );
  if ( id.isEmpty() )
  {
    const auto& obj = context->stash( "object" );
    if ( !obj.isNull() ) id = obj.toHash().value( "id" ).toString();
  }

  auto query = CPreparedSqlQueryThreadForDB( 
    "update agreements set filename=:filename, mimetype=:mimetype, filesize=:filesize, document=:document, checksum=:checksum, updated=:updated where agreement_id=:id",
    crrc::DATABASE_NAME );
  auto bytes = bindAgreement( context, query );
  query.bindValue( ":id", id.toInt() );

  if ( query.exec() )
  {
    auto agreement = agreementFromContext( context, bytes );
    auto aid = id.toUInt();
    agreement.id = aid;

    std::lock_guard<std::mutex> lock{ agreementMutex };
    agreements[aid] = std::move( agreement );
  }
  else context->stash()["error_msg"] = query.lastError().text();
}

QVariantList AgreementDAO::search( Cutelyst::Context* context, const Mode& mode ) const
{
  const auto text = context->request()->param( "text", "" );
  const QString clause = "%" % text % "%";

  auto query = CPreparedSqlQueryThreadForDB(
   "select agreement_id from agreements where filename like :text order by filename",
    DATABASE_NAME );

  query.bindValue( ":text", clause );
  QVariantList list;

  if ( query.exec() )
  {
    while ( query.next() )
    {
      auto agreement = retrieve( query.value( 0 ).toString(), mode );
      list.append( agreement );
    }
  }
  else
  {
    qWarning() << query.lastError().text();
    context->stash()["error_msg"] = query.lastError().text();
  }

  return list;
}

QString AgreementDAO::remove( uint32_t id ) const
{
  auto query = CPreparedSqlQueryThreadForDB(
    "delete from agreements where agreement_id = :id", DATABASE_NAME );
  query.bindValue( ":id", id );
  if ( query.exec() )
  {
    std::lock_guard<std::mutex> lock{ agreementMutex };
    agreements.remove( id );
    return QString::number( query.numRowsAffected() );
  }

  return query.lastError().text();
}

QString AgreementDAO::saveProgram( Cutelyst::Context* context ) const
{
  const auto func = [=]()
  {
    auto obj = agreements.find( context->request()->param( "agreement_id" ).toUInt() );
    if ( obj != agreements.end() )
    {
      auto& value = obj.value();
      value.transferInstitutionId = context->request()->param( "transfer_institution_id" );
      value.transfereeInstitutionId = context->request()->param( "transferee_institution_id" );
    }
  };

  auto query = CPreparedSqlQueryThreadForDB(
    "update agreements set transfer_institution_id = :i, transferee_institution_id = :ei where agreement_id = :id",
    DATABASE_NAME );
  query.bindValue( ":i", context->request()->param( "transfer_institution_id" ).toUInt() );
  query.bindValue( ":ei", context->request()->param( "transferee_institution_id" ).toUInt() );
  query.bindValue( ":id", context->request()->param( "agreement_id" ).toUInt() );

  if ( query.exec() )
  {
    if ( query.numRowsAffected() > 0 )
    {
      func();
      return QString();
    }

    query = CPreparedSqlQueryThreadForDB(
      "insert into agreements (transfer_institution_id, transferee_institution_id, agreement_id) values (:i, :ei, :id)",
      DATABASE_NAME );
    query.bindValue( ":i", context->request()->param( "transfer_institution_id" ).toUInt() );
    query.bindValue( ":ei", context->request()->param( "transferee_institution_id" ).toUInt() );
    query.bindValue( ":id", context->request()->param( "agreement_id" ).toUInt() );

    if ( query.exec() )
    {
      if ( query.numRowsAffected() > 0 )
      {
        func();
        return QString();
      }
      return QString( "0" );
    }
  }

  return query.lastError().text();
}
