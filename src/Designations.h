#pragma once

#include <Cutelyst/Controller>

namespace crrc
{
  class Designations : public Cutelyst::Controller
  {
    Q_OBJECT

  public:
    explicit Designations( QObject* parent = nullptr ) : Controller( parent ) {}
    ~Designations() = default;

    C_ATTR( index, :Path( "/designations" ) : Args( 0 ) )
    void index( Cutelyst::Context* c );

    C_ATTR( base, :Chained( "/" ) : PathPart( "designations" ) : CaptureArgs( 0 ) )
    void base( Cutelyst::Context* c ) const;

    C_ATTR( object, :Chained( "base" ) : PathPart( "id" ) : CaptureArgs( 1 ) )
    void object( Cutelyst::Context* c, const QString& id ) const;

    C_ATTR( data, :Chained( "object" ) : PathPart( "data" ) : Args( 0 ) )
    void data( Cutelyst::Context* c ) const;

    C_ATTR( save, :Chained( "base" ) : PathPart( "save" ) : Args( 0 ) )
    void save( Cutelyst::Context* c ) const;

    C_ATTR( remove, :Chained( "base" ) : PathPart( "remove" ) : Args( 0 ) )
    void remove( Cutelyst::Context* c ) const;
  };
}

