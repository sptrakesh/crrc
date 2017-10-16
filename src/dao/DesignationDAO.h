#pragma once

#include <Cutelyst/context.h>

namespace crrc
{
  namespace dao
  {
    struct DesignationDAO
    {
      QVariantList retrieveAll() const;
      QVariant retrieve( const uint32_t id ) const;
      QVariantList retrieveByType( const QString& type ) const;
      uint32_t insert( Cutelyst::Context* context ) const;
      uint32_t update( Cutelyst::Context* context ) const;
      uint32_t remove( uint32_t id ) const;
    };
  }
}
