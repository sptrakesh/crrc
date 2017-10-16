#pragma once

#include <memory>
#include <QtCore/QJsonObject>
#include <QtSql/QSqlQuery>
#include <Cutelyst/Context>

namespace crrc
{
  namespace model
  {
    class Designation : public QObject
    {
      Q_OBJECT
      Q_PROPERTY( QVariant id READ getDesignationId )
      Q_PROPERTY( QString type READ getType )
      Q_PROPERTY( QString title READ getTitle )

    public:
      using Ptr = std::unique_ptr<Designation>;
      static Ptr create( QSqlQuery& query );
      static Ptr create( Cutelyst::Context* context );

      static const Designation* from( const QVariant& variant )
      {
        return qvariant_cast<Designation*>( variant );
      }

      explicit Designation( QObject* parent = nullptr ) : QObject( parent ) {}
      ~Designation() = default;

      uint32_t getId() const { return id; }
      QVariant getDesignationId() const { return QVariant{ id }; }
      Designation& setId( uint32_t id )
      {
        this->id = id;
        return *this;
      }

      const QString& getType() const { return type; }
      const QString& getTitle() const { return title; }

      operator QString() const;

    private:
      uint32_t id = 0;
      QString type;
      QString title;
    };

    inline QVariant asVariant( const Designation* designation )
    {
      return QVariant::fromValue<QObject*>( const_cast<Designation*>( designation ) );
    }

    inline bool operator< ( const Designation& des1, const Designation& des2 )
    {
      return des1.getTitle() < des2.getTitle();
    }

    QJsonObject toJson( const Designation& designation );
  }
}
