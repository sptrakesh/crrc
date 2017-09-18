#pragma once

#include <Cutelyst/Controller>

namespace crrc
{
  class InstitutionDesignations : public Cutelyst::Controller
  {
    Q_OBJECT
  public:
    explicit InstitutionDesignations( QObject* parent = nullptr ) : Controller( parent ) {}
    ~InstitutionDesignations() = default;

    C_ATTR( index, :Path( "/institution/designations" ) : Args( 0 ) )
    void index( Cutelyst::Context* c );

    C_ATTR( base, :Chained( "/" ) : PathPart( "institution/designations" ) : CaptureArgs( 0 ) )
    void base( Cutelyst::Context* c ) const;

    C_ATTR( object, :Chained( "base" ) : PathPart( "id" ) : CaptureArgs( 1 ) )
    void object( Cutelyst::Context* c, const QString& id ) const;

    C_ATTR( view, :Chained( "object" ) : PathPart( "view" ) : Args( 0 ) )
    void view( Cutelyst::Context* c ) const;

    C_ATTR( edit, :Chained( "object" ) : PathPart( "edit" ) : Args( 0 ) )
    void edit( Cutelyst::Context* c ) const;

    C_ATTR( save, :Chained( "base" ) : PathPart( "save" ) : Args( 0 ) )
    void save( Cutelyst::Context* c ) const;

    C_ATTR( remove, :Chained( "base" ) : PathPart( "remove" ) : Args( 0 ) )
    void remove( Cutelyst::Context* c ) const;

  private:
    void sendJson( Cutelyst::Context* c, const QString& result ) const;
  };
}
