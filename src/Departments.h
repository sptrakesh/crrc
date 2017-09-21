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

    C_ATTR( create, :Chained( "base" ) : PathPart( "create" ) : Args( 0 ) )
    void create( Cutelyst::Context* c ) const;

    C_ATTR( checkUnique, :Chained( "base" ) : PathPart( "checkUnique" ) : Args( 0 ) )
    void checkUnique( Cutelyst::Context* c );
  };
}
