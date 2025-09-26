#pragma once
#include "MathAlias.h"
#include "../HelperFiles/MyConcepts.h"
#include <random>
namespace nest
{
    struct Random
    {
        std::mt19937 m_kEngine;
        unsigned int m_seed;
        Random(unsigned int seed = 0)
            : m_seed(seed)
        {
            if (m_seed != 0)
            {
                m_kEngine.seed(m_seed);
            }
            else
            {
                std::random_device randomDevice;
                auto randomSeed = randomDevice();
                m_kEngine.seed(randomSeed);
                m_seed = randomSeed;
            }
        }

        template <FloatingPoint NumberType>
        NumberType GenerateFloatNumInRange(NumberType min, NumberType max)
        {
            std::uniform_real_distribution<NumberType> distribution(min, max);
            return distribution(m_kEngine);
        }

        template <Integral NumberType>
        NumberType GenerateIntegralNumInRange(NumberType min, NumberType max)
        {
            std::uniform_int_distribution<NumberType> distribution(min, max);
            return distribution(m_kEngine);
        }

        template <Arithmetic NumberType>
        Vec2 GenerateRandomVector(NumberType min, NumberType max)
        {
            std::uniform_int_distribution<NumberType> distribution(min, max);
            NumberType x = distribution(m_kEngine);
            NumberType y = distribution(m_kEngine);
            Vec2 vector = Vec2(x, y);
            return vector;
        }
    };
}