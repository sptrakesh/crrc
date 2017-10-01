#pragma once

#include <Cutelyst/context.h>

namespace crrc
{
  namespace dao
  {
    struct InstitutionDAO
    {
      QVariantList retrieveAll() const;
      QVariant retrieve( uint32_t id ) const;
      uint32_t insert( Cutelyst::Context* context ) const;
      void update( Cutelyst::Context* context ) const;
      QVariantList search( Cutelyst::Context* context ) const;
      uint32_t remove( uint32_t id ) const;
      bool isUnique( const QString& name, const QString& city ) const;
    };
  }
}