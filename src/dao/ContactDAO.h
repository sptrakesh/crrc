#pragma once
#include <Cutelyst/context.h>

namespace crrc
{
  namespace dao
  {
    struct ContactDAO
    {
      QVariantList retrieveAll() const;
      QVariant retrieve( const uint32_t id ) const;
      QVariant retrieveByUser( uint32_t id ) const;
      QVariantList retrieveByInstitution( uint32_t id ) const;
      uint32_t insert( Cutelyst::Context* context ) const;
      void update( Cutelyst::Context* context ) const;
      QVariantList search( Cutelyst::Context* context ) const;
      uint32_t remove( uint32_t id ) const;
      void removeInstitution( uint32_t id ) const;
    };
  }
}