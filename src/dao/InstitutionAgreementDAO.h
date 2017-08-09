#pragma once
#include <Cutelyst/context.h>

class QSqlQuery;


namespace crrc
{
  namespace dao
  {
    struct InstitutionAgreementDAO
    {
      QVariantList retrieve( const QString& agreementId ) const;
      uint32_t insert( Cutelyst::Context* context ) const;
      void remove( Cutelyst::Context* context ) const;

    private:
      QVariantList retrieveByAgreement( const QString& agreementId ) const;
      QVariantList retrieveByKey( const QStringList& ids ) const;
      QVariantList toList( QSqlQuery& query ) const;
    };
  }
}
