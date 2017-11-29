#pragma once
#include "BaseTest.h"

namespace crrc
{
  class ContactTest : public BaseTest
  {
    Q_OBJECT
  public:
    ContactTest( QObject* parent = nullptr ) : BaseTest( parent ),
        name{ "UnitTestContact" }, workEmail{ "testcontact@unittest.com" },
        username{ "unittestuser" }, password{ "Unit5@stUser" },
      title{ "Unit Test Title" } {}

    private slots :
    void initTestCase();
    void create();
    void retrieveAll();
    void retrieve();
    void invalid();
    void checkUsername();
    void checkInvalidUsername();
    void index();
    void loginBeforeEdit();
    void update();
    void loginAfterEdit();
    void byInstitution();
    void search();
    void remove();
    void readDeleted();
    void cleanupTestCase();

  private:
    const QString name;
    const QString workEmail;
    const QString username;
    const QString password;
    const QString title;
    const uint32_t roleId = 3;
  };
}

DECLARE_TEST( crrc::ContactTest )
