#pragma once
#include "AutoTest.h"
#include "BaseTest.h"

namespace crrc
{
  class DegreeTest : public BaseTest
  {
    Q_OBJECT
  public:
    DegreeTest( QObject* parent = nullptr ) : BaseTest( parent ),
      title{ "UnitTestDegree" }, duration{ "degreeDurationValue" } {}

  private slots:
    void create();
    void retrieve();
    void invalid();
    void update();
    void remove();

  private:
    QString title;
    QString duration;
  };
}

DECLARE_TEST( crrc::DegreeTest )