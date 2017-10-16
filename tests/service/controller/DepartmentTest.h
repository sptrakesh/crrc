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

    void create();
    void retrieve();
    void invalid();
    void index();
    void update();
    void retrieveModified();
    void remove();
    void readDeleted();

  private:
    QString name;
    QString prefix;
  };
}

DECLARE_TEST( crrc::DepartmentTest )
