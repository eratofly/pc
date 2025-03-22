#pragma once

#include <iostream>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>
#include <shared_mutex>
#include "BankOperationErrors.h"

using AccountId = unsigned long long;
using Money = long long;

class Bank
{
public:
    explicit Bank(Money initialCash)
            : m_cash(initialCash)
    {
        if (initialCash < 0)
        {
            throw BankOperationError("Initial cash cannot be negative");
        }
    }

    unsigned long long GetOperationsCount() const
    {
        return m_operationsCount.load();
    }

    void SendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount)
    {
        if (amount < 0)
        {
            throw std::out_of_range("Amount cannot be negative");
        }

        std::shared_lock lock(m_mutex);
        ++m_operationsCount;


        if (m_accounts.find(srcAccountId) == m_accounts.end() || m_accounts.find(dstAccountId) == m_accounts.end())
        {
            throw BankOperationError("Invalid account ID");
        }

        if (m_accounts[srcAccountId] < amount)
        {
            throw BankOperationError("Insufficient funds");
        }

        m_accounts[srcAccountId] -= amount;
        m_accounts[dstAccountId] += amount;
    }

    bool TrySendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount)
    {
        if (amount < 0)
        {
            throw std::out_of_range("Amount cannot be negative");
        }

        std::shared_lock lock(m_mutex);
        if (m_accounts.find(srcAccountId) == m_accounts.end() || m_accounts.find(dstAccountId) == m_accounts.end())
        {
            throw BankOperationError("Invalid account ID");
        }

        if (m_accounts[srcAccountId] < amount)
        {
            return false;
        }

//        std::scoped_lock lockTransfer(m_mutex);

        m_accounts[srcAccountId] -= amount;
        m_accounts[dstAccountId] += amount;
        ++m_operationsCount;


        return true;
    }

    Money GetCash() const
    {
        return m_cash.load();
    }

    Money GetAccountBalance(AccountId accountId)
    {
        std::shared_lock lock(m_mutex);

        if (m_accounts.find(accountId) == m_accounts.end())
        {
            throw BankOperationError("Invalid account ID");
        }

        ++m_operationsCount;

        return m_accounts.at(accountId);
    }

    void WithdrawMoney(AccountId accountId, Money amount)
    {
        if (amount < 0)
        {
            throw std::out_of_range("Amount cannot be negative");
        }

        std::shared_lock lock(m_mutex);

        if (m_accounts.find(accountId) == m_accounts.end())
        {
            throw BankOperationError("Invalid account ID");
        }

        if (m_accounts[accountId] < amount)
        {
            throw BankOperationError("Insufficient funds");
        }

        m_accounts[accountId] -= amount;
        std::shared_lock lockCash(m_mutexCash);
        m_cash += amount;

        ++m_operationsCount;
    }

    bool TryWithdrawMoney(AccountId accountId, Money amount)
    {
        if (amount < 0)
        {
            throw std::out_of_range("Amount cannot be negative");
        }

        std::shared_lock lock(m_mutex);
        if (m_accounts.find(accountId) == m_accounts.end())
        {
            throw BankOperationError("Invalid account ID");
        }

        if (m_accounts[accountId] < amount)
        {
            return false;
        }

        m_accounts[accountId] -= amount;
        std::shared_lock lockCash(m_mutexCash);
        m_cash += amount;
        ++m_operationsCount;

        return true;
    }

    void DepositMoney(AccountId accountId, Money amount)
    {
        if (amount < 0)
        {
            throw std::out_of_range("Amount cannot be negative");
        }

        std::shared_lock lock(m_mutex);

        if (m_accounts.find(accountId) == m_accounts.end())
        {
            throw BankOperationError("Invalid account ID");
        }

        if (m_cash < amount)
        {
            throw BankOperationError("Insufficient m_cash");
        }

        std::shared_lock lockCash(m_mutexCash);
        m_cash -= amount;
        m_accounts[accountId] += amount;

        ++m_operationsCount;
    }

    AccountId OpenAccount()
    {
        std::shared_lock lock(m_mutex);
        AccountId newAccountId = m_nextAccountId++;
        m_accounts[newAccountId] = 0;
        ++m_operationsCount;
        return newAccountId;
    }

    Money CloseAccount(AccountId accountId)
    {
        std::shared_lock lock(m_mutex);
        if (m_accounts.find(accountId) == m_accounts.end())
        {
            throw BankOperationError("Invalid account ID");
        }

        Money balance = m_accounts[accountId];
        std::shared_lock lockCash(m_mutexCash);
        m_cash += balance;
        m_accounts.erase(accountId);
        ++m_operationsCount;
        return balance;
    }

private:
    std::atomic<unsigned long long> m_operationsCount = {0};
    std::unordered_map<AccountId, Money> m_accounts;
    std::atomic<Money> m_cash;
    mutable std::shared_mutex m_mutex;
    mutable std::shared_mutex m_mutexCash;
    AccountId m_nextAccountId = 1;
};