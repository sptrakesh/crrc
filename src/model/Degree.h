#pragma once

#include <memory>
#include <QtCore/QJsonObject>
#include <QtSql/QSqlQuery>
#include <Cutelyst/Context>

namespace crrc
{
  namespace model
  {
    class Degree : public QObject
    {
      Q_OBJECT
      Q_PROPERTY( QVariant id READ getDegreeId )
      Q_PROPERTY( QString title READ getTitle )
      Q_PROPERTY( QString duration READ getDuration )

    public:
      using Ptr = std::unique_ptr<Degree>;
      static Ptr create( QSqlQuery& query );
      static Ptr create( Cutelyst::Context* context );

      static const Degree* from( const QVariant& variant )
      {
        return qvariant_cast<Degree*>( variant );
      }

      explicit Degree( QObject* parent = nullptr ) : QObject( parent ) {}
      ~Degree() = default;

      uint32_t getId() const { return id; }
      QVariant getDegreeId() const { return QVariant{ id }; }

      Degree& setId( uint32_t id )
      {
        this->id = id;
        return *this;
      }

      const QString& getTitle() const { return title; }
      const QString& getDuration() const { return duration; }

      operator QString() const;

    private:
      uint32_t id = 0;
      QString title;
      QString duration;
    };

    inline QVariant asVariant( const Degree* degree )
    {
      return QVariant::fromValue<QObject*>( const_cast<Degree*>( degree ) );
    }

    inline bool operator< ( const Degree& deg1, const Degree& deg2 )
    {
      return deg1.getTitle() < deg2.getTitle();
    }

    QJsonObject toJson( const Degree& degree );
  }
}
