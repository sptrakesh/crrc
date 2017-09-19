#pragma once
#include <Cutelyst/context.h>

namespace crrc
{
  namespace dao
  {
    struct AgreementDAO
    {
      enum class Mode : char { Full, Partial };
      QVariantList retrieveAll( const Mode& mode = Mode::Full ) const;
      QVariantHash retrieve( const QString& id, const Mode& mode = Mode::Full ) const;
      QByteArray contents( Cutelyst::Context* context, const QString& id ) const;
      uint32_t insert( Cutelyst::Context* context ) const;
      void update( Cutelyst::Context* context ) const;
      QVariantList search( Cutelyst::Context* context, const Mode& mode = Mode::Full ) const;
      QString remove( uint32_t id ) const;
      QString saveProgram( Cutelyst::Context* context ) const;
    };
  }
}