#pragma once

#include <Cutelyst/Application>


namespace crrc
{
  class AdminApplication : public Cutelyst::Application
  {
    Q_OBJECT
    CUTELYST_APPLICATION( IID "crrc::AdminApplication" )
  public:
    Q_INVOKABLE explicit AdminApplication( QObject* parent = nullptr ) :
        Cutelyst::Application( parent ) {}
    ~AdminApplication() = default;

    bool init();
    bool postFork() override;
  };
}