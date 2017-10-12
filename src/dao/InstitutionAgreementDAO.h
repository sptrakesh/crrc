#pragma once

#include <QtSql/QSqlQuery>
#include <Cutelyst/context.h>

namespace crrc
{
  namespace dao
  {
    struct InstitutionAgreementDAO
    {
      QVariantList retrieve( Cutelyst::Context* context, const QString& agreementId ) const;
      uint32_t insert( Cutelyst::Context* context ) const;
      void remove( Cutelyst::Context* context ) const;

    private:
      QVariantList retrieveByAgreement( Cutelyst::Context* context, const uint32_t agreementId ) const;
      QVariantList retrieveByKey( Cutelyst::Context* context, const QStringList& ids ) const;
      QVariantList toList( Cutelyst::Context* context, QSqlQuery& query ) const;
    };
  }
}
