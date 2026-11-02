#include <JuceHeader.h>
#include <gtest/gtest.h>

#include "common/Tolerances.h"

#include <cstring>
#include <iostream>

namespace
{
    // Prints every tolerance and its grid as JSON, for the experiment scripts
    void listTolerances()
    {
        using namespace tapsynth::test;

        juce::Array<juce::var> list;
        for (const auto* t : tol::all)
        {
            auto* obj = new juce::DynamicObject();
            obj->setProperty ("name", t->name);
            obj->setProperty ("unit", t->unit);
            obj->setProperty ("bound", t->bound == Bound::atMost ? "atMost" : "atLeast");
            obj->setProperty ("value", t->value);

            juce::Array<juce::var> grid;
            for (const auto g : t->grid)
                grid.add (g);

            obj->setProperty ("grid", grid);
            list.add (juce::var (obj));
        }

        std::cout << juce::JSON::toString (list).toStdString() << std::endl;
    }
}

int main (int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
        if (std::strcmp (argv[i], "--tapsynth-list-tolerances") == 0)
        {
            listTolerances();
            return 0;
        }

    // JUCE's message manager must exist for the processor's parameters and listeners
    juce::ScopedJuceInitialiser_GUI juceInit;

    ::testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS();
}
