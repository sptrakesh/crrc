#pragma once

#include "BaseTest.h"

namespace crrc
{
  class InstitutionDesignationTest : public BaseTest
  {
    Q_OBJECT
  public:
    InstitutionDesignationTest( QObject* parent = nullptr ) : BaseTest( parent ) {}

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void create();
    void retrieve();
    void update();
    void retrieveModified();
    void remove();
    void retrieveAfterDelete();

  private:
    void createInstitution();
    void createDesignation();
    void removeInstitution();
    void removeDesignation();

  private:
    uint32_t expiration = 2017;
  };
}

DECLARE_TEST( crrc::InstitutionDesignationTest )
