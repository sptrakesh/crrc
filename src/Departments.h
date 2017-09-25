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

    C_ATTR( index, :Path( "/institution/departments" ) : Args( 0 ) )
    void index( Cutelyst::Context* c );

    C_ATTR( base, :Chained( "/" ) : PathPart( "institution/departments" ) : CaptureArgs( 0 ) )
    void base( Cutelyst::Context* ) const;

    C_ATTR( object, :Chained( "base" ) : PathPart( "id" ) : CaptureArgs( 1 ) )
    void object( Cutelyst::Context* c, const QString& id ) const;

    C_ATTR( list, :Chained( "object" ) : PathPart( "list" ) : Args( 0 ) )
    void list( Cutelyst::Context* c ) const;

    C_ATTR( save, :Chained( "base" ) : PathPart( "save" ) : Args( 0 ) )
    void save( Cutelyst::Context* c ) const;

    C_ATTR( remove, :Chained( "base" ) : PathPart( "remove" ) : Args( 0 ) )
    void remove( Cutelyst::Context* c ) const;
  };
}
