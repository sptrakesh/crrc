#pragma once

#include <memory>
#include <QtCore/QJsonObject>
#include <QtSql/QSqlQuery>
#include <Cutelyst/Context>

namespace crrc
{
  namespace model
  {
    class Department : public QObject
    {
      Q_OBJECT
      Q_PROPERTY( QVariant id READ getDepartmentId )
      Q_PROPERTY( QString name READ getName )
      Q_PROPERTY( QString prefix READ getPrefix )
      Q_PROPERTY( QVariant institution READ getInstitution )

    public:
      using Ptr = std::unique_ptr<Department>;
      static Ptr create( QSqlQuery& query );
      static Ptr create( Cutelyst::Context* context );

      static Department* from( const QVariant& var )
      {
        return qvariant_cast<Department*>( var );
      }

      explicit Department( QObject* parent = nullptr ) : QObject( parent ), id{ 0 } {}
      ~Department() = default;

      uint32_t getId() const { return id; }
      QVariant getDepartmentId() const { return QVariant{ id }; }

      Department& setId( uint32_t id )
      {
        this->id = id;
        return *this;
      }

      const QString& getName() const { return name; }
      const QString& getPrefix() const { return prefix; }
      uint32_t getInstitutionId() const { return institutionId; }
      QVariant getInstitution() const;

    private:
      uint32_t id = 0;
      QString name;
      QString prefix;
      uint32_t institutionId = 0;
    };

    inline QVariant asVariant( const Department* department )
    {
      return QVariant::fromValue<QObject*>( const_cast<Department*>( department ) );
    }

    inline bool operator< ( const Department& deg1, const Department& deg2 )
    {
      return deg1.getName() < deg2.getName();
    }

    QJsonObject toJson( const Department& department );
  }
}