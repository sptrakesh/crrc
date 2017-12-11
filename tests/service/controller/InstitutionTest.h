#pragma  once

#include "BaseTest.h"

namespace crrc
{
  class InstitutionTest : public BaseTest
  {
    Q_OBJECT
  public:
    InstitutionTest( QObject* parent = nullptr ) : BaseTest( parent ),
    name{ "Unit Test Institution" }, city{ "Unit Test City" } {}

  private slots:
    void create();
    void retrieve();
    void invalid();
    void checkUnique();
    void checkInvalidUnique();
    void index();
    void update();
    void remove();

  private:
    QString name;
    QString city;
    uint32_t institutionTypeId = 1;
  };
}

DECLARE_TEST( crrc::InstitutionTest )
