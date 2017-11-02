#pragma once

#include "BaseTest.h"

namespace crrc
{
  class ProgramTest : public BaseTest
  {
    Q_OBJECT
  public:
    ProgramTest( QObject* parent = nullptr ) : BaseTest( parent ),
        title{ "Unit Test Program" }, credits{ "3" } {}

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void create();
    void retrieve();
    void invalid();
    void index();
    void update();
    void retrieveModified();
    void searchInstitutions();
    void search();
    void remove();
    void retrieveAfterDelete();

  private:
    void createInstitution();
    void createDegree();
    void removeInstitution();
    void removeDegree();

  private:
    QString title;
    QString credits;
  };
}

DECLARE_TEST( crrc::ProgramTest )
