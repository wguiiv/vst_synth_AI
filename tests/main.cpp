#include <JuceHeader.h>
#include <gtest/gtest.h>

int main (int argc, char** argv)
{
    // JUCE's message manager must exist for the processor's parameters and listeners
    juce::ScopedJuceInitialiser_GUI juceInit;

    ::testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS();
}
