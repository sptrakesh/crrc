#pragma once
#include "BaseTest.h"

namespace crrc
{
  class InstitutionTypeTest : public BaseTest
  {
    Q_OBJECT
  public:
    InstitutionTypeTest( QObject* parent = nullptr ) : BaseTest( parent ) {}

  private slots:
    void index();
    void retrieve();
  };
}

DECLARE_TEST( crrc::InstitutionTypeTest )
