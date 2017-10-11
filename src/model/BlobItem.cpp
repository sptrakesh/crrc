#include "BlobItem.h"
#include "dao/functions.h"

#include <QtCore/QFileInfo>
#include <Cutelyst/Upload>

using crrc::model::BlobItem;

void BlobItem::populate( QSqlQuery& query )
{
  id = query.value( 0 ).toUInt();
  filename = query.value( 1 ).toString();
  mimetype = query.value( 2 ).toString();
  filesize = query.value( 3 ).toUInt();
  checksum = query.value( 4 ).toString();
  updated = query.value( 5 ).toString();
}

void BlobItem::populate( Cutelyst::Context* context, const QByteArray& bytes )
{
  const auto upload = context->request()->upload( "file" );
  const QFileInfo file{ upload->filename() };
  filename = file.fileName();
  mimetype = upload->contentType();
  filesize = upload->size();
  checksum = dao::checksum( bytes );
  updated = dao::httpDate( file.lastModified() );
}

QJsonObject crrc::model::toJson( const BlobItem& blob )
{
  QJsonObject obj;
  obj.insert( "id", static_cast<int>( blob.getId() ) );
  obj.insert( "filename", blob.getFilename() );
  obj.insert( "mimetype", blob.getMimetype() );
  obj.insert( "filesize", static_cast<int>( blob.getFilesize() ) );
  obj.insert( "checksum", blob.getChecksum() );
  obj.insert( "updated", blob.getUpdated() );
  return obj;
}
