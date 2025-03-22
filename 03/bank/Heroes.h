#pragma once

#include <random>
#include "CharacterBase.h"
#include "CharacterWithBankAccount.h"

class Homer : public CharacterWithBankAccount
{
public:
    Homer(Money cash, AllCharacters &characters, bool logger, Bank &bank, AccountId accountId)
            : CharacterWithBankAccount(cash, characters, logger, bank, accountId)
    {}

    void Run(const AllCharacters &characters) override
    {
        if (SendMoneySomebody(*characters.marge, 100))
        {
            Log("Homer send money to Marge");
        }

        if (SendCashSomebody(*characters.bart, 10))
        {
            Log("Homer send cash to Bart");
        }

        if (SendCashSomebody(*characters.lisa, 10))
        {
            Log("Homer send cash to Lisa");
        }
    }

};

class Marge : public CharacterWithBankAccount
{
public:
    Marge(Money cash, AllCharacters &characters, bool logger, Bank &bank, AccountId accountId)
            : CharacterWithBankAccount(cash, characters, logger, bank, accountId)
    {}

    void Run(const AllCharacters &characters) override
    {
        if (SendMoneySomebody(*characters.apu, 50))
        {
            Log("Marge has bough products");
        }
    }

};

class BartLisa : public CharacterBase
{
public:
    BartLisa(Money cash, AllCharacters &characters, bool logger)
            : CharacterBase(cash, characters, logger)
    {}

    void Run(const AllCharacters &characters) override
    {
        if (SendCashSomebody(*characters.apu, 50))
        {
            Log("Sibling has bough mini products");
        }
    }

};

class Apu : public CharacterWithBankAccount
{
public:
    Apu(Money cash, AllCharacters &characters, bool logger, Bank &bank, AccountId accountId)
            : CharacterWithBankAccount(cash, characters, logger, bank, accountId)
    {}

    void Run(const AllCharacters &characters) override
    {
        if (SendMoneySomebody(*characters.burns, 200))
        {
            Log("Apu has paid for electricity");
        }
        if (DepositMoneyByCharacter(GetCash()))
        {
            Log("Apu has deposited All Cash");
        }
    }
};

class Burns : public CharacterWithBankAccount
{
public:
    Burns(Money cash, AllCharacters &characters, bool logger, Bank &bank, AccountId accountId)
            : CharacterWithBankAccount(cash, characters, logger, bank, accountId)
    {}

    void Run(const AllCharacters &characters) override
    {
        if (SendMoneySomebody(*characters.homer, 150))
        {
            Log("Burns has paid salary");
        }
    }
};

class Nelson : public CharacterBase
{
public:
    Nelson(Money cash, AllCharacters &characters, bool logger)
            : CharacterBase(cash, characters, logger)
    {}

    void Run(const AllCharacters &characters) override
    {
        if (!characters.bart) return;

        Money stealAmount = GenerateRandom(0, 20);
        if (SendCashSomebody(*characters.bart, stealAmount))
        {
            Log("Nelson has stolen " + std::to_string(stealAmount) + " from Bart");

            if (characters.apu && SendCashSomebody(*characters.apu, stealAmount))
            {
                Log("Nelson has bought cigarettes from Apu");
            }
        }
    }

private:
    static Money GenerateRandom(Money min, Money max)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<Money> dis(min, max);

        return dis(gen);
    }
};

class Snake : public CharacterWithBankAccount
{
public:
    Snake(Money cash, AllCharacters &characters, bool logger, Bank &bank, AccountId accountId)
            : CharacterWithBankAccount(cash, characters, logger, bank, accountId)
    {}

    void Run(const AllCharacters &characters) override
    {
        if (!characters.homer) return;

        Money hackAmount = GenerateRandom(50, 150);
        if (SendMoneySomebody(*characters.homer, hackAmount))
        {
            Log("Snake transferred " + std::to_string(hackAmount) + " from Homer's account");

            if (characters.apu && SendMoneySomebody(*characters.apu, hackAmount / 2))
            {
                Log("Snake bought cigarettes from Apu");
            }
        }
    }

private:
    static Money GenerateRandom(Money min, Money max)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<Money> dis(min, max);

        return dis(gen);
    }
};

class Smithers : public CharacterWithBankAccount
{
public:
    Smithers(Money cash, AllCharacters &characters, bool logger, Bank &bank, AccountId accountId)
            : CharacterWithBankAccount(cash, characters, logger, bank, accountId)
    {}

    void Run(const AllCharacters &characters) override
    {
        m_accountCounter++;

        if (m_accountCounter % 5 == 0)
        {
            try
            {
                Money balance = m_bank.CloseAccount(GetAccountId());
                m_cash += balance;
                m_accountId = m_bank.OpenAccount();
                m_accountValid = true;
                Log("Smithers opened a new account");
            } catch (...)
            {
                m_accountValid = false;
            }
        }

        if (m_accountValid && characters.burns)
        {
            if (characters.burns->SendMoneySomebody(*this, 200))
            {
                Log("Smithers got salary");
            }
        }

        if (m_accountValid && characters.apu)
        {
            if (SendMoneySomebody(*characters.apu, 100))
            {
                Log("Smithers bought groceries from Apu");
            }
        }
    }

private:
    bool m_accountValid = true;
    int m_accountCounter = 0;
};