#pragma once

#include <QtCore/QDebug>
#include <QtSql/QSqlQuery>
#include <Cutelyst/Context>

namespace crrc
{
  namespace model
  {
    class InstitutionAgreement : public QObject
    {
      Q_OBJECT
      Q_PROPERTY( QVariant agreement READ getAgreement )
      Q_PROPERTY( QVariant transferProgram READ getTransferProgram )
      Q_PROPERTY( QVariant transfereeProgram READ getTransfereeProgram )

    public:
      static InstitutionAgreement* create( Cutelyst::Context* context, QSqlQuery& query );

      static const InstitutionAgreement* from( const QVariant& variant )
      {
        return qvariant_cast<InstitutionAgreement*>( variant );
      }

      explicit InstitutionAgreement( QObject* parent = nullptr ) : QObject( parent )
      {
        qDebug() << "Created object: " << this;
      }

      ~InstitutionAgreement()
      {
        qDebug() << "Deleting object: " << this;
      }

      uint32_t getAgreementId() const { return agreementId; }
      QVariant getAgreement() const;

      InstitutionAgreement& setAgreementId( uint32_t id )
      {
        this->agreementId = id;
        return *this;
      }

      uint32_t getTransferProgramId() const { return transferProgramId; }
      QVariant getTransferProgram() const;

      uint32_t getTransfereeProgramId() const { return transfereeProgramId; }
      QVariant getTransfereeProgram() const;

    private:
      uint32_t agreementId = 0;
      uint32_t transferProgramId = 0;
      uint32_t transfereeProgramId = 0;
    };

    inline QVariant asVariant( const InstitutionAgreement* degree )
    {
      return QVariant::fromValue<QObject*>( const_cast<InstitutionAgreement*>( degree ) );
    }
  }
}
