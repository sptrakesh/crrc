#pragma once
#include <Cutelyst/Controller>


namespace crrc
{
  class Login : public Cutelyst::Controller
  {
    Q_OBJECT
  public:
    explicit Login( QObject* parent = nullptr );
    ~Login();

    C_ATTR( index, :Path("/login"):Args( 0 ) )
    void index( Cutelyst::Context* c );

  private:
    bool plainText( Cutelyst::Context* c );
    bool hashed( Cutelyst::Context* c );
    bool login( Cutelyst::Context* c );
    void service( Cutelyst::Context* c );
  };
}
