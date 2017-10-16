#pragma once

#include "BaseTest.h"

namespace crrc
{
  class DepartmentTest : public BaseTest
  {
    Q_OBJECT
  public:
    DepartmentTest( QObject* parent = nullptr ) : BaseTest( parent ),
        name{ "UnitTestDepartment" }, prefix{ "utd" } {}

  private slots:
    void initTestCase();
    void cleanupTestCase();

  private:
    QString name;
    QString prefix;
  };
}

