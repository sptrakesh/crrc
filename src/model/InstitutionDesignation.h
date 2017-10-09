#pragma once

#include <QtCore/QDebug>
#include <QtSql/QSqlQuery>
#include <Cutelyst/Context>

namespace crrc
{
  namespace model
  {
    class InstitutionDesignation : public QObject
    {
      Q_OBJECT
      Q_PROPERTY( QVariant institution READ getInstitution )
      Q_PROPERTY( QVariant designation READ getDesignation )
      Q_PROPERTY( QVariant expiration READ getExpirationVariant )

    public:
      static InstitutionDesignation* create(
        Cutelyst::Context* context, QSqlQuery& query );

      static const InstitutionDesignation* from( const QVariant& variant )
      {
        return qvariant_cast<InstitutionDesignation*>( variant );
      }

      explicit InstitutionDesignation( QObject* parent = nullptr ) : QObject( parent )
      {
        qDebug() << "Created object: " << this;
      }

      ~InstitutionDesignation()
      {
        qDebug() << "Deleting object: " << this;
      }

      uint32_t getInstitutionId() const { return institutionId; }
      QVariant getInstitution() const;

      InstitutionDesignation& setInstitutionId( uint32_t id )
      {
        this->institutionId = id;
        return *this;
      }

      uint32_t getDesignationId() const { return designationId; }
      QVariant getDesignation() const;

      uint32_t getExpiration() const { return expiration; }
      QVariant getExpirationVariant() const { return QVariant( static_cast<int>( expiration ) ); }

    private:
      uint32_t institutionId = 0;
      uint32_t designationId = 0;
      uint32_t expiration = 0;
    };

    inline QVariant asVariant( const InstitutionDesignation* degree )
    {
      return QVariant::fromValue<QObject*>( const_cast<InstitutionDesignation*>( degree ) );
    }
  }
}
