#pragma once

#include <Cutelyst/Controller>

namespace crrc
{
  class InstitutionTypes : public Cutelyst::Controller
  {
  public:
    Q_OBJECT
  public:
    explicit InstitutionTypes( QObject* parent = nullptr ) : Controller( parent ) {}
    ~InstitutionTypes() = default;

    C_ATTR(index, :Path("/institutionTypes") :AutoArgs)
    void index( Cutelyst::Context* c ) const;

    C_ATTR( base, :Chained( "/" ) : PathPart( "institutionTypes" ) : CaptureArgs( 0 ) )
    void base( Cutelyst::Context* ) const;

    C_ATTR( object, :Chained( "base" ) : PathPart( "id" ) : CaptureArgs( 1 ) )
    void object( Cutelyst::Context* c, const QString& id ) const;

    C_ATTR( data, :Chained( "object" ) : PathPart( "data" ) : Args( 0 ) )
    void data( Cutelyst::Context* c ) const;
  };
}
