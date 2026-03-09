/*
g++ -std=c++23 -Wall -Wextra 08.04.cpp -o 08.04.out
./08.04.out
*/

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <random>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////

class WeaselSimulation
{
public:
    WeaselSimulation();
    void run();

private:
    char randomLetter();

    std::string generateInitialString();
    std::string mutate(const std::string& parent);
    std::size_t computeDistance(const std::string& s) const;

    const std::string m_target;                 
    const double m_mutationRate;                 // probability to mutate a character 
    const std::size_t m_populationSize;          // number of offspring per generation 
    const std::size_t m_stringLength;            // length of target 

    std::default_random_engine m_engine;         
    std::uniform_int_distribution<int> m_letterDist;    // uniform [0,25] for letters a‑z
    std::uniform_real_distribution<double> m_mutationDist; // uniform [0,1) for mutation decisions

    std::string m_current;                        // current parent string
};

///////////////////////////////////////////////////////////////////////////////////////////

WeaselSimulation::WeaselSimulation()
    : m_target("methinksitislikeaweasel")
    , m_mutationRate(0.05)
    , m_populationSize(100)
    , m_stringLength(m_target.size())
    , m_engine(std::random_device{}())
    , m_letterDist(0, 25)
    , m_mutationDist(0.0, 1.0)
    , m_current(generateInitialString())
{
}

///////////////////////////////////////////////////////////////////////////////////////////

char WeaselSimulation::randomLetter()
{
    return static_cast<char>('a' + m_letterDist(m_engine));
}

///////////////////////////////////////////////////////////////////////////////////////////

std::string WeaselSimulation::generateInitialString()
{
    std::string result;
    result.reserve(m_stringLength);
    for (std::size_t i = 0; i < m_stringLength; ++i)
        result.push_back(randomLetter());
    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////

std::string WeaselSimulation::mutate(const std::string& parent)
{
    std::string offspring(parent);
    for (char& c : offspring)
        if (m_mutationDist(m_engine) < m_mutationRate)
            c = randomLetter();
    return offspring;
}

///////////////////////////////////////////////////////////////////////////////////////////

std::size_t WeaselSimulation::computeDistance(const std::string& s) const
{
    std::size_t distance = 0;
    for (std::size_t i = 0; i < m_stringLength; ++i)
        if (s[i] != m_target[i])
            ++distance;
    return distance;
}

///////////////////////////////////////////////////////////////////////////////////////////

void WeaselSimulation::run()
{
    std::size_t generation = 0;

    while (true)
    {
        std::cout << m_current << '\n';
        if (computeDistance(m_current) == 0)
            break;

        std::vector<std::string> offspring;
        offspring.reserve(m_populationSize);
        std::vector<std::size_t> distances;
        distances.reserve(m_populationSize);

        for (std::size_t i = 0; i < m_populationSize; ++i)
        {
            std::string child = mutate(m_current);
            std::size_t dist = computeDistance(child);
            offspring.push_back(std::move(child));
            distances.push_back(dist);
        }

        bool perfectFound = false;
        for (std::size_t i = 0; i < m_populationSize; ++i)
        {
            if (distances[i] == 0)
            {
                std::cout << offspring[i] << '\n';
                perfectFound = true;
                break;
            }
        }
        if (perfectFound)
            break;

        auto minIt = std::min_element(distances.begin(), distances.end());
        std::size_t bestIndex = std::distance(distances.begin(), minIt);
        m_current = std::move(offspring[bestIndex]);

        ++generation;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    WeaselSimulation simulation;
    simulation.run();

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////