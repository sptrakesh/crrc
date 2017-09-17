#pragma once
#include <QtCore/QStringBuilder>
#include <QtCore/QCryptographicHash>
#include <QtCore/QDateTime>
#include <QtCore/QLocale>


namespace crrc
{
  namespace dao
  {
    inline QString checksum( const QByteArray& bytes )
    {
      return QString::fromLatin1(
        QCryptographicHash::hash( bytes, QCryptographicHash::Md5 ).toBase64() );
    }

    inline QString httpDate( const QDateTime& date )
    {
      return QLocale::c().toString( date,
        QLatin1String( "ddd, dd MMM yyyy hh:mm:ss 'GMT'" ) ).toLatin1();
    }
  }
}
