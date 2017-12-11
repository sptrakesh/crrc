#pragma once

#include <memory>
#include <QtCore/QJsonObject>
#include <QtSql/QSqlQuery>
#include <Cutelyst/Context>

namespace crrc
{
  namespace model
  {
    class Institution : public QObject
    {
      Q_OBJECT
      Q_PROPERTY( QVariant id READ getInstitutionId )
      Q_PROPERTY( QString name READ getName )
      Q_PROPERTY( QString address READ getAddress )
      Q_PROPERTY( QString city READ getCity )
      Q_PROPERTY( QString state READ getState )
      Q_PROPERTY( QString postalCode READ getPostalCode )
      Q_PROPERTY( QString country READ getCountry )
      Q_PROPERTY( QString website READ getWebsite )
      Q_PROPERTY( QVariant logo READ getLogo )
      Q_PROPERTY( QVariant institutionType READ getInstitutionType )

    public:
      using Ptr = std::unique_ptr<Institution>;
      static Ptr create( QSqlQuery& query );
      static Ptr create( Cutelyst::Context* context );

      static const Institution* from( const QVariant& variant )
      {
        return qvariant_cast<Institution*>( variant );
      }

      explicit Institution( QObject* parent = nullptr ) : QObject( parent ) {}
      ~Institution() = default;

      uint32_t getId() const { return id; }
      QVariant getInstitutionId() const { return QVariant{ id }; }

      Institution& setId( uint32_t id )
      {
        this->id = id;
        return *this;
      }

      const QString& getName() const { return name; }
      const QString& getAddress() const { return address; }
      const QString& getCity() const { return city; }
      const QString& getState() const { return state; }
      const QString& getPostalCode() const { return postalCode; }
      const QString& getCountry() const { return country; }
      const QString& getWebsite() const { return website; }

      uint32_t getLogoId() const { return logoId; }
      QVariant getLogo() const;

      uint32_t getInstitutionTypeId() const { return institutionTypeId; }
      QVariant getInstitutionType() const;

    private:
      uint32_t id = 0;
      QString name;
      QString address;
      QString city;
      QString state;
      QString postalCode;
      QString country;
      QString website;
      uint32_t logoId = 0;
      uint32_t institutionTypeId = 0;
    };

    inline QVariant asVariant( const Institution* degree )
    {
      return QVariant::fromValue<QObject*>( const_cast<Institution*>( degree ) );
    }

    inline bool operator< ( const Institution& inst1, const Institution& inst2 )
    {
      return inst1.getName() < inst2.getName();
    }

    QJsonObject toJson( const Institution& institution, bool compact = false );
  }
}
