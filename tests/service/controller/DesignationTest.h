#pragma once
#include "BaseTest.h"

namespace crrc
{
  class DesignationTest : public BaseTest
  {
    Q_OBJECT
  public:
    DesignationTest( QObject* parent = nullptr ) : BaseTest( parent ),
        title{ "UnitTestDesignation" }, type{ "UT" } {}

  private slots:
    void create();
    void retrieve();
    void invalid();
    void index();
    void update();
    void checkModified();
    void remove();
    void readDeleted();

  private:
    QString title;
    QString type;
  };
}

DECLARE_TEST( crrc::DesignationTest )
