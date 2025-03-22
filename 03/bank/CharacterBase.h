#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <syncstream>
#include "Bank.h"

class CharacterBase;
class CharacterWithBankAccount;

struct AllCharacters
{
    CharacterWithBankAccount *homer = nullptr;
    CharacterWithBankAccount *marge = nullptr;
    CharacterBase *lisa = nullptr;
    CharacterBase *bart = nullptr;
    CharacterWithBankAccount *apu = nullptr;
    CharacterWithBankAccount *burns = nullptr;
    CharacterBase* nelson = nullptr;
    CharacterWithBankAccount* snake = nullptr;
    CharacterWithBankAccount* smithers = nullptr;
};

class CharacterBase
{
public:
    //Money &cash ????????
    CharacterBase(Money cash, AllCharacters &characters, bool logger)
            : m_cash(cash), m_characters(characters), m_logger(logger)
    {}

    virtual void Run(AllCharacters const &characters) = 0;

    Money GetCash() const
    {
        std::shared_lock lockCash(m_mutexCash);
        return m_cash;
    }

//    Money AddCash(Money amount)
//    {
//        std::shared_lock lock(m_mutex);
//        m_cash += amount;
//    }

    bool SendCashSomebody(CharacterBase &character, Money amount)
    {
        auto& hisMutex = character.m_mutex;
        std::scoped_lock lk{hisMutex, m_mutex};
        if (m_cash < amount)
        {
            return false;
        }
        m_cash -= amount;
        character.m_cash += amount;
        return true;
    }

    void Log(std::string const &str) const
    {
        if (m_logger)
        {
            std::osyncstream out(std::cout);
            out << str << std::endl;
        }
    }

    virtual ~CharacterBase() = default;

private:
    AllCharacters &m_characters;
    bool m_logger;
    mutable std::shared_mutex m_mutex;
    mutable std::shared_mutex m_mutexCash;
protected:
    Money m_cash;
};

