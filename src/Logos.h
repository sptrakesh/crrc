#pragma once

#include <Cutelyst/Controller>

namespace crrc
{
  class Logos : public Cutelyst::Controller
  {
    Q_OBJECT
  public:
    explicit Logos( QObject* parent = nullptr ) {}
    ~Logos() = default;

    C_ATTR( base, :Chained( "/" ) : PathPart( "logos" ) : CaptureArgs( 0 ) )
    void base( Cutelyst::Context* ) const;

    C_ATTR( object, :Chained( "base" ) : PathPart( "id" ) : CaptureArgs( 1 ) )
    void object( Cutelyst::Context* c, const QString& id ) const;

    C_ATTR( create, :Chained( "base" ) : PathPart( "create" ) : Args( 0 ) )
    void create( Cutelyst::Context* c ) const;

    C_ATTR( update, :Chained( "object" ) : PathPart( "update" ) : Args( 0 ) )
    void update( Cutelyst::Context* c ) const { create( c ); }

    C_ATTR( display, :Chained( "object" ) : PathPart( "display" ) : Args( 0 ) )
    void display( Cutelyst::Context* c ) const;

    C_ATTR( remove, :Chained( "base" ) : PathPart( "remove" ) : Args( 0 ) )
    void remove( Cutelyst::Context* c );
  };
}
