#pragma once

#include <memory>
#include <QtCore/QJsonObject>
#include <QtSql/QSqlQuery>
#include <Cutelyst/Context>

namespace crrc
{
  namespace model
  {
    class Program : public QObject
    {
      Q_OBJECT
      Q_PROPERTY( QVariant id READ getProgramId )
      Q_PROPERTY( QString title READ getTitle )
      Q_PROPERTY( QString credits READ getCredits )
      Q_PROPERTY( QString type READ getType )
      Q_PROPERTY( QString curriculumCode READ getCurriculumCode )
      Q_PROPERTY( QString url READ getUrl )
      Q_PROPERTY( QVariant institution READ getInstitution )
      Q_PROPERTY( QVariant degree READ getDegree )
      Q_PROPERTY( QVariant designation READ getDesignation )

    public:
      using Ptr = std::unique_ptr<Program>;
      static Ptr create( QSqlQuery& query );
      static Ptr create( Cutelyst::Context* context );

      static const Program* from( const QVariant& variant )
      {
        return qvariant_cast<Program*>( variant );
      }

      explicit Program( QObject* parent = nullptr ) : QObject( parent ) {}
      ~Program() = default;

      uint32_t getId() const { return id; }
      QVariant getProgramId() const { return QVariant{ id }; }

      Program& setId( uint32_t id )
      {
        this->id = id;
        return *this;
      }

      const QString& getTitle() const { return title; }
      const QString& getCredits() const { return credits; }
      const QString& getType() const { return type; }
      const QString& getCurriculumCode() const { return curriculumCode; }
      const QString& getUrl() const { return url; }

      uint32_t getInstitutionId() const { return institutionId; }
      QVariant getInstitution() const;

      uint32_t getDegreeId() const { return degreeId; }
      QVariant getDegree() const;

      uint32_t getDesignationId() const { return designationId; }
      QVariant getDesignation() const;

    private:
      uint32_t id = 0;
      QString title;
      QString credits;
      QString type;
      QString curriculumCode;
      QString url;
      uint32_t institutionId = 0;
      uint32_t degreeId = 0;
      uint32_t designationId = 0;
    };

    inline QVariant asVariant( const Program* program )
    {
      return QVariant::fromValue<QObject*>( const_cast<Program*>( program ) );
    }

    bool operator< ( const Program& lhs, const Program& rhs );

    QJsonObject toJson( const Program& program, bool compact = false );
  }
}
