#pragma once

#include <Cutelyst/context.h>

namespace crrc
{
  namespace dao
  {
    struct DegreeDAO
    {
      QVariantList retrieveAll() const;
      QVariantHash retrieve( const QString& id ) const;
      uint32_t insert( Cutelyst::Context* context ) const;
      void update( Cutelyst::Context* context ) const;
      QVariantList search( Cutelyst::Context* context ) const;
      uint32_t remove( uint32_t id ) const;
    };
  }
}