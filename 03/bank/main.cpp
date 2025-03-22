#include <iostream>
#include <csignal>
#include <atomic>
#include <vector>
#include <thread>
#include "Heroes.h"

std::atomic<bool> StopSimulation(false);

void SignalHandler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        StopSimulation.store(true);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <simulation_time_seconds> <parallel|single>" << std::endl;
        return 1;
    }

    int simulationTime = std::stoi(argv[1]);
    std::string mode = argv[2];

    Bank bank(10000);
    AllCharacters characters;

    AccountId homerAccount = bank.OpenAccount();
    AccountId margeAccount = bank.OpenAccount();
    AccountId apuAccount = bank.OpenAccount();
    AccountId burnsAccount = bank.OpenAccount();
    AccountId snakeAccount = bank.OpenAccount();
    AccountId smithersAccount = bank.OpenAccount();

    bool logger = false;

    Homer homer(2000, characters, logger, bank, homerAccount);
    Marge marge(1000, characters, logger, bank, margeAccount);
    BartLisa bartLisa(0, characters, logger);
    Apu apu(4000, characters, logger, bank, apuAccount);
    Burns burns(3000, characters, logger, bank, burnsAccount);
    Nelson nelson(0, characters, logger);
    Snake snake(0, characters, logger, bank, snakeAccount);
    Smithers smithers(0, characters, logger, bank, smithersAccount);

    characters.homer = &homer;
    characters.marge = &marge;
    characters.bart = &bartLisa;
    characters.lisa = &bartLisa;
    characters.apu = &apu;
    characters.burns = &burns;
    characters.nelson = &nelson;
    characters.snake = &snake;
    characters.smithers = &smithers;

    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    auto startTime = std::chrono::steady_clock::now();

    if (mode == "parallel")
    {
        std::vector<std::thread> threads;
        threads.emplace_back([&]()
                             { while (!StopSimulation.load()) homer.Run(characters); });
        threads.emplace_back([&]()
                             { while (!StopSimulation.load()) marge.Run(characters); });
        threads.emplace_back([&]()
                             { while (!StopSimulation.load()) bartLisa.Run(characters); });
        threads.emplace_back([&]()
                             { while (!StopSimulation.load()) apu.Run(characters); });
        threads.emplace_back([&]()
                             { while (!StopSimulation.load()) burns.Run(characters); });

        threads.emplace_back([&]()
                             { while (!StopSimulation.load()) nelson.Run(characters); });
        threads.emplace_back([&]()
                             { while (!StopSimulation.load()) snake.Run(characters); });
        threads.emplace_back([&]()
                             { while (!StopSimulation.load()) smithers.Run(characters); });

        std::this_thread::sleep_for(std::chrono::seconds(simulationTime));
        StopSimulation.store(true);


        for (auto &thread: threads)
        {
            if (thread.joinable()) thread.join();
        }
    }
    else if (mode == "single")
    {
        while (std::chrono::steady_clock::now() - startTime < std::chrono::seconds(simulationTime))
        {
            homer.Run(characters);
            marge.Run(characters);
            bartLisa.Run(characters);
            apu.Run(characters);
            burns.Run(characters);
            nelson.Run(characters);
            snake.Run(characters);
            smithers.Run(characters);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    Money totalMoney = homer.GetCash() + marge.GetCash() + bartLisa.GetCash() + apu.GetCash() + burns.GetCash() + nelson.GetCash() + snake.GetCash() + smithers.GetCash();

    for (auto &account: {homerAccount, margeAccount, apuAccount, burnsAccount, snakeAccount, smithers.GetAccountId()})
    {
        totalMoney += bank.GetAccountBalance(account);
    }

    std::cout << "Total operations: " << bank.GetOperationsCount() << std::endl;
    std::cout << "Total totalCash: " << totalMoney << std::endl;
    std::cout << "Consistency check: " << (totalMoney == 10000 ? "OK" : "FAIL") << std::endl;

    return 0;
}