#include "Check.h"

#include <cmath>
#include <limits>
#include <sstream>

namespace tapsynth::test
{
    ::testing::AssertionResult within (const Tolerance& t, double measured, const std::string& label)
    {
        std::ostringstream value;
        value.precision (9);
        value << measured;
        ::testing::Test::RecordProperty (std::string (t.name) + "|" + label, value.str());

        const auto pass = t.bound == Bound::atMost ? measured <= t.value : measured >= t.value;

        if (pass)
            return ::testing::AssertionSuccess();

        return ::testing::AssertionFailure()
               << label << ": " << t.name << " measured " << value.str() << " " << t.unit
               << (t.bound == Bound::atMost ? ", limit <= " : ", limit >= ") << t.value << " " << t.unit;
    }

    double noteHz (double midiNote)
    {
        return 440.0 * std::pow (2.0, (midiNote - 69.0) / 12.0);
    }

    double cents (double measuredHz, double expectedHz)
    {
        if (measuredHz <= 0.0 || expectedHz <= 0.0)
            return std::numeric_limits<double>::infinity();

        return 1200.0 * std::log2 (measuredHz / expectedHz);
    }
}
