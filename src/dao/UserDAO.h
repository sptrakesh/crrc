#pragma once
#include <Cutelyst/context.h>

namespace crrc
{
  namespace dao
  {
    struct UserDAO
    {
      QVariantList retrieveAll() const;
      QVariantHash retrieve( const QString& id ) const;
      QVariantHash retrieveByUsername( const QString& username ) const;
      uint32_t insert( Cutelyst::Context* context ) const;
      void update( Cutelyst::Context* context ) const;
      QVariantList search( Cutelyst::Context* context ) const;
      QString remove( uint32_t id ) const;
      bool isUsernameAvailable( const QString& username ) const;
      bool updateRole( uint32_t userId, uint32_t roleId ) const;
      bool updatePassword( const QString& username, const QString& password ) const;
    };
  }
}
