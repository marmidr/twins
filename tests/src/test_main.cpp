/******************************************************************************
 * @brief   TWins - unit tests main
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *****************************************************************************/

#include "gtest/gtest.h"
#include "twins.hpp"
#include "twins_pal_defimpl.hpp"

#include <vector>
#include <string>
#include <memory>

// -----------------------------------------------------------------------------

namespace twins
{

const char* encodeClTheme(ColorFG /* cl */)  { return ""; }
const char* encodeClTheme(ColorBG /* cl */)  { return ""; }
ColorFG intensifyClTheme(ColorFG cl) { return cl; }
ColorBG intensifyClTheme(ColorBG cl) { return cl; }

}

// -----------------------------------------------------------------------------

// must be global due to static twins objects destroyed after main() quit
struct TestPAL : twins::DefaultPAL
{
    TestPAL()
    {
        // call once to test no-pal version of log()
        twins::log(nullptr, __FILE__, __LINE__, nullptr, nullptr);
        twins::log(nullptr, __FILE__, __LINE__, nullptr, "");
        // self-register:
        twins::init(this);
    }

    ~TestPAL()
    {
        fprintf(stderr, "~TestPAL\n");
        deinit();
        // replaces this pal with the dummy one - cause mem leaks
        twins::deinit();
    }

    void flushBuff() override
    {
        // do not write anything to terminal
        if (lineBuff.size())
        {
            if (lineBuff.size() > lineBuffMaxSize)
                lineBuffMaxSize = lineBuff.size();

            lineBuff.clear();
        }
    }
};

// -----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    std::vector<char*> vargs(argv, argv+argc);

#ifdef GTEST_NOCOLOR // from CMake
    std::string color = "--gtest_color=no";
    vargs.push_back((char*)color.c_str());
#else
    std::string color = "--gtest_color=yes";
    vargs.push_back((char*)color.c_str());
#endif

    argc = vargs.size();
    testing::InitGoogleTest(&argc, vargs.data());

    int rc{};

    {
        auto test_pal = std::make_unique<TestPAL>();
        twins::mouseMode(twins::MouseMode::M1);
        rc = RUN_ALL_TESTS();
        twins::mouseMode(twins::MouseMode::Off);
        fprintf(stderr, "\n");
        fprintf(stderr, "*** Tests finished ***\n");
    }

    return rc;
}
