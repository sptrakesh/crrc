#pragma once

#include <QtCore/QCoreApplication>
#include <QtTest/QTest>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QSharedPointer>

namespace AutoTest
{
  using TestList = QList<QObject*>;

  inline TestList& testList()
  {
    static TestList list;
    return list;
  }

  inline bool findObject( QObject* object )
  {
    auto& list = testList();
    if ( list.contains( object ) ) return true;

    foreach( QObject* test, list )
    {
      if ( test->objectName() == object->objectName() ) return true;
    }
    return false;
  }

  inline void addTest( QObject* object )
  {
    auto& list = testList();
    if ( !findObject( object ) )
    {
      list.append( object );
    }
  }

  inline int run( int argc, char *argv[] )
  {
    QMap<QObject*, int> map;
    auto ret = 0;

    foreach( QObject* test, testList() )
    {
      const auto result = QTest::qExec( test, argc, argv );
      if ( result ) map[test] = result;
      ret += result;
    }

    qDebug() << "Finished running " << testList().size() << 
      " tests cases with " << ret << " failures.";

    for ( auto it = map.cbegin(); it != map.cend(); ++it )
    {
      qWarning() << "Test case: " << it.key()->objectName() << " had " << it.value() << "failure(s).";
    }
    return ret;
  }
}

template <class T>
class Test
{
public:
  QSharedPointer<T> child;

  Test( const QString& name ) : child( new T )
  {
    child->setObjectName( name );
    AutoTest::addTest( child.data() );
  }
};

#define CONCAT_(x,y) x##y
#define CONCAT(x,y) CONCAT_(x,y)
#define DECLARE_TEST(className) static Test<className> CONCAT(t, __COUNTER__)(#className);

#define TEST_MAIN \
    int main(int argc, char *argv[]) \
    { \
      QCoreApplication a(argc, argv); \
      return AutoTest::run(argc, argv); \
    }

