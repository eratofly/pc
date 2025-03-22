#pragma once
#include "CharacterBase.h"

class CharacterWithBankAccount : public CharacterBase
{
public:
    CharacterWithBankAccount(Money cash, AllCharacters &characters, bool logger, Bank &bank, AccountId accountId)
            : CharacterBase(cash, characters, logger), m_bank(bank), m_accountId(accountId)
    {}

    bool SendMoneySomebody(CharacterWithBankAccount& character, Money amount)
    {
        std::scoped_lock lock(m_mutex, character.m_mutex);        return m_bank.TrySendMoney(character.GetAccountId(), m_accountId, amount);
    }

    bool WithDrawMoneyByCharacter(Money amount)
    {
        if (m_bank.TryWithdrawMoney(m_accountId, amount))
        {
            std::shared_lock lockCash(m_mutexCash);
            m_cash += amount;
            return true;
        }

        return false;
    }

    bool DepositMoneyByCharacter(Money amount)
    {
        std::shared_lock lockCash(m_mutexCash);
        if (m_cash >= amount)
        {
            m_bank.DepositMoney(m_accountId, amount);
            m_cash -= amount;
            return true;
        }

        return false;
    }

    AccountId GetAccountId() const
    {
        std::shared_lock lock(m_mutex);
        return m_accountId;
    }


private:
    mutable std::shared_mutex m_mutex;
    mutable std::shared_mutex m_mutexCash;
protected:
    AccountId m_accountId;
    Bank &m_bank;
};