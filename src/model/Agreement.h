#pragma once

#include "BlobItem.h"
#include <memory>

namespace crrc
{
  namespace model
  {
    class Agreement : public BlobItem
    {
      Q_OBJECT
      Q_PROPERTY( QVariant transferInstitution READ getTransferInstitution )
      Q_PROPERTY( QVariant transfereeInstitution READ getTransfereeInstitution )

    public:
      using Ptr = std::unique_ptr<Agreement>;
      static Ptr create( QSqlQuery& query );
      static Ptr create( Cutelyst::Context* context, const QByteArray& bytes );

      static const Agreement* from( const QVariant& variant )
      {
        return qvariant_cast<Agreement*>( variant );
      }

      explicit Agreement( QObject* parent = nullptr ) : BlobItem( parent ) {}
      ~Agreement() = default;

      uint32_t getTransferInstitutionId() const { return transferInstitutionId; }
      QVariant getTransferInstitution() const;

      uint32_t getTransfereeInstitutionId() const { return transfereeInstitutionId; }
      QVariant getTransfereeInstitution() const;

      Agreement& updatePrograms( Cutelyst::Context* context );

    private:
      uint32_t transferInstitutionId = 0;
      uint32_t transfereeInstitutionId = 0;
    };

    inline QVariant asVariant( const Agreement* agreement )
    {
      return QVariant::fromValue<QObject*>( const_cast<Agreement*>( agreement ) );
    }

    QJsonObject toJson( const Agreement& agreement );
  }
}
