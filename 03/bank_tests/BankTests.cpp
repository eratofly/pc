#define CATCH_CONFIG_MAIN
#include "../bank/Bank.h"
#include "../lib/catch2/catch.hpp"

TEST_CASE("Bank initialization and basic operations") {
    Bank bank(10000);

    SECTION("Initial cash amount") {
        REQUIRE(bank.GetCash() == 10000);
    }

    SECTION("Open account and check balance") {
        AccountId id = bank.OpenAccount();
        REQUIRE(bank.GetAccountBalance(id) == 0);
    }

    SECTION("Deposit money to account") {
        AccountId id = bank.OpenAccount();
        bank.DepositMoney(id, 500);
        REQUIRE(bank.GetAccountBalance(id) == 500);
        REQUIRE(bank.GetCash() == 9500);
    }

    SECTION("Withdraw money from account") {
        AccountId id = bank.OpenAccount();
        bank.DepositMoney(id, 500);
        bank.WithdrawMoney(id, 200);
        REQUIRE(bank.GetAccountBalance(id) == 300);
        REQUIRE(bank.GetCash() == 9700);
    }
}

TEST_CASE("Money transfers") {
    Bank bank(10000);
    AccountId account1 = bank.OpenAccount();
    AccountId account2 = bank.OpenAccount();
    bank.DepositMoney(account1, 1000);

    SECTION("Successful transfer between accounts") {
        bank.SendMoney(account1, account2, 500);
        REQUIRE(bank.GetAccountBalance(account1) == 500);
        REQUIRE(bank.GetAccountBalance(account2) == 500);
    }

    SECTION("Insufficient funds for transfer") {
        REQUIRE_THROWS_AS(bank.SendMoney(account1, account2, 1500), BankOperationError);
    }

    SECTION("Invalid account in transfer") {
        REQUIRE_THROWS_AS(bank.SendMoney(9999, account2, 100), BankOperationError);
    }
}

TEST_CASE("Account management") {
    Bank bank(10000);
    AccountId acc = bank.OpenAccount();
    bank.DepositMoney(acc, 800);

    SECTION("Close account with balance") {
        Money balance = bank.CloseAccount(acc);
        REQUIRE(balance == 800);
        REQUIRE(bank.GetCash() == 10000);
        REQUIRE_THROWS_AS(bank.GetAccountBalance(acc), BankOperationError);
    }

    SECTION("Try close non-existent account") {
        REQUIRE_THROWS_AS(bank.CloseAccount(9999), BankOperationError);
    }
}

TEST_CASE("Error handling") {
    Bank bank(10000);
    AccountId acc = bank.OpenAccount();

    SECTION("Negative amount operations") {
        REQUIRE_THROWS_AS(bank.DepositMoney(acc, -100), std::out_of_range);
        REQUIRE_THROWS_AS(bank.WithdrawMoney(acc, -50), std::out_of_range);
        REQUIRE_THROWS_AS(bank.SendMoney(acc, acc, -10), std::out_of_range);
    }

    SECTION("Insufficient cash for deposit") {
        REQUIRE_THROWS_AS(bank.DepositMoney(acc, 20000), BankOperationError);
    }
}

TEST_CASE("Consistency checks") {
    Bank bank(10000);
    AccountId account1 = bank.OpenAccount();
    AccountId account2 = bank.OpenAccount();

    bank.DepositMoney(account1, 3000);
    bank.DepositMoney(account2, 2000);
    bank.WithdrawMoney(account1, 1000);
    bank.SendMoney(account2, account1, 500);

    SECTION("Total money conservation") {
        Money total_cash = bank.GetCash();
        Money total_accounts = bank.GetAccountBalance(account1) + bank.GetAccountBalance(account2);
        REQUIRE(total_cash + total_accounts == 10000);
    }
}