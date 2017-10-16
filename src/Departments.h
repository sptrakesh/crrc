#pragma once

#include <Cutelyst/Controller>

namespace crrc
{
  class Departments : public Cutelyst::Controller
  {
    Q_OBJECT
  public:
    explicit Departments( QObject* parent = nullptr ) : Controller( parent ) {}
    ~Departments() = default;

    C_ATTR( index, :Path( "/departments" ) : Args( 0 ) )
    void index( Cutelyst::Context* c );

    C_ATTR( base, :Chained( "/" ) : PathPart( "departments" ) : CaptureArgs( 0 ) )
    void base( Cutelyst::Context* ) const;

    C_ATTR( object, :Chained( "base" ) : PathPart( "id" ) : CaptureArgs( 1 ) )
    void object( Cutelyst::Context* c, const QString& id ) const;

    C_ATTR( data, :Chained( "object" ) : PathPart( "data" ) : Args( 0 ) )
    void data( Cutelyst::Context* c ) const;
  };
}
