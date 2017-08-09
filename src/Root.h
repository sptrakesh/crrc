#pragma once

#include <Cutelyst/Controller>


namespace crrc
{
  class Root : public Cutelyst::Controller
  {
    Q_OBJECT
    C_NAMESPACE( "" )
  public:
    explicit Root( QObject* parent = 0 );
    ~Root();

    C_ATTR( index, :Path:Args( 0 ) )
    void index( Cutelyst::Context* c );

    C_ATTR( defaultPage, :Path )
    void defaultPage( Cutelyst::Context* c );

  private:
    /**
     * Check if there is a user and, if not, forward to login page
     */
    C_ATTR( Auto, :Private )
    bool Auto( Cutelyst::Context* c );

    C_ATTR( End, :ActionClass( "RenderView" ) )

    void End( Cutelyst::Context* c )
    {
      Q_UNUSED( c );
    }
  };
}
