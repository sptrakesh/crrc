#pragma once

#include <memory>
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

      explicit Degree( QObject* parent = nullptr ) : QObject( parent ), id{ 0 } {}
      ~Degree() = default;

      uint32_t getId() const { return id; }
      QVariant getDegreeId() const { return id; }

      Degree& setId( uint32_t id )
      {
        this->id = id;
        return *this;
      }

      const QString& getTitle() const { return title; }
      const QString& getDuration() const { return duration; }

      operator QString() const;

    private:
      uint32_t id;
      QString title;
      QString duration;
    };

    inline QVariant asVariant( const Degree* degree )
    {
      return QVariant::fromValue<QObject*>( const_cast<Degree*>( degree ) );
    }
  }
}
