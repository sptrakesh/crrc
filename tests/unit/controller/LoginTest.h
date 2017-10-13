#pragma once
#include "AutoTest.h"

namespace crrc
{
  class LoginTest : public QObject
  {
    Q_OBJECT
  private slots:
    void redirect();
    void valid();
    void invalid();
  };
}

DECLARE_TEST( crrc::LoginTest )
