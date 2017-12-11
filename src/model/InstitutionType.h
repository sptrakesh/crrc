#pragma once

#include <memory>
#include <QtCore/QJsonObject>
#include <QtSql/QSqlQuery>
#include <Cutelyst/Context>

namespace crrc
{
  namespace model
  {
    class InstitutionType : public QObject
    {
      Q_OBJECT
      Q_PROPERTY( QVariant id READ getInstitutionTypeId )
      Q_PROPERTY( QString name READ getName )

    public:
      using Ptr = std::unique_ptr<InstitutionType>;
      static Ptr create( QSqlQuery& query );

      static const InstitutionType* from( const QVariant& variant )
      {
        return qvariant_cast<InstitutionType*>( variant );
      }

      explicit InstitutionType( QObject* parent = nullptr ) : QObject( parent ) {}
      ~InstitutionType() = default;

      uint32_t getId() const { return id; }
      QVariant getInstitutionTypeId() const { return QVariant{ id }; }

      InstitutionType& setId( uint32_t id )
      {
        this->id = id;
        return *this;
      }

      const QString& getName() const { return name; }

    private:
      uint32_t id = 0;
      QString name;
    };

    inline QVariant asVariant( const InstitutionType* instType )
    {
      return QVariant::fromValue<QObject*>( const_cast<InstitutionType*>( instType ) );
    }

    inline bool operator< ( const InstitutionType& instType1, const InstitutionType& instType2 )
    {
      return instType1.getName() < instType2.getName();
    }

    QJsonObject toJson( const InstitutionType& instType );
  }
}
