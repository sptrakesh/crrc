#pragma once

#include <Cutelyst/Controller>


namespace crrc
{
  class Logout : public Cutelyst::Controller
  {
    Q_OBJECT
  public:
    explicit Logout( QObject* parent = 0 );
    ~Logout();

    C_ATTR( index, :Path("/logout"):Args( 0 ) )
    void index( Cutelyst::Context* c );
  };
}
