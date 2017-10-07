#pragma once

#include <QtSql/QSqlQuery>
#include <Cutelyst/Context>

namespace crrc
{
  namespace model
  {
    class BlobItem : public QObject
    {
      Q_OBJECT
      Q_PROPERTY( QVariant id READ getBlobItemId )
      Q_PROPERTY( QString filename READ getFilename )
      Q_PROPERTY( QString mimetype READ getMimetype )
      Q_PROPERTY( QVariant filesize READ getFileSize )
      Q_PROPERTY( QString updated READ getUpdated )

    public:
      static const BlobItem* from( const QVariant& variant )
      {
        return qvariant_cast<BlobItem*>( variant );
      }

      explicit BlobItem( QObject* parent = nullptr ) : QObject( parent ) {}
      ~BlobItem() = default;

      uint32_t getId() const { return id; }
      QVariant getBlobItemId() const { return QVariant{ id }; }

      BlobItem& setId( uint32_t id )
      {
        this->id = id;
        return *this;
      }

      const QString& getFilename() const { return filename; }
      const QString& getMimetype() const { return mimetype; }
      uint32_t getFilesize() const { return filesize; }
      QVariant getFileSize() const { return QVariant{ filesize }; }
      const QString& getChecksum() const { return checksum; }
      const QString& getUpdated() const { return updated; }

      void populate( QSqlQuery& query );
      void populate( Cutelyst::Context* context, const QByteArray& bytes );

    private:
      uint32_t id = 0;
      QString filename;
      QString mimetype;
      uint32_t filesize = 0;
      QString checksum;
      QString updated;
    };
  }
}
