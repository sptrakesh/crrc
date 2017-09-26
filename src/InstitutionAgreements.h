#pragma once
#include <Cutelyst/Controller>


namespace crrc
{
  class InstitutionAgreements : public Cutelyst::Controller
  {
    Q_OBJECT
  public:
    explicit InstitutionAgreements( QObject* parent = nullptr ) : Controller( parent ) {}
    ~InstitutionAgreements() = default;

    C_ATTR(index, :Path("/institution/program/relations") :AutoArgs)
    void index( Cutelyst::Context* c ) const;

    C_ATTR( base, :Chained( "/" ) : PathPart( "institution/program/relations" ) : CaptureArgs( 0 ) )
    void base( Cutelyst::Context* ) const;

    C_ATTR( object, :Chained( "base" ) : PathPart( "id" ) : CaptureArgs( 1 ) )
    void object( Cutelyst::Context* c, const QString& id ) const;

    C_ATTR( edit, :Chained( "base" ) : PathPart( "edit" ) : Args( 0 ) )
    void edit( Cutelyst::Context* c ) const;

    C_ATTR( remove, :Chained( "base" ) : PathPart( "remove" ) : Args( 0 ) )
    void remove( Cutelyst::Context* c );

  private:
    bool validate( Cutelyst::Context* context ) const;
  };
}
