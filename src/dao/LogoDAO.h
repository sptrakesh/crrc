#pragma once

#include <Cutelyst/context.h>

namespace crrc
{
  namespace dao
  {
    struct LogoDAO
    {
      QVariantList retrieveAll() const;
      QVariant retrieve( const uint32_t id ) const;
      QByteArray contents( Cutelyst::Context* context, const uint32_t id ) const;
      uint32_t insert( Cutelyst::Context* context ) const;
      uint32_t update( Cutelyst::Context* context ) const;
      uint32_t remove( uint32_t id ) const;
    };
  }
}
