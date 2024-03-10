#define UNIT_TEST
#define CATCH_CONFIG_MAIN
#include <sstream>
#include "catch.hpp"
#include "../src/buggy.cxx"

// Helper function to set up cin for a test
void setupCin(const std::string& input) {
    static std::istringstream iss;
    iss.str(input);
    std::cin.rdbuf(iss.rdbuf());
}

// Helper function to set up cin and cout for a test
void setupIo(const std::string& input, std::ostringstream& output, std::streambuf*& originalCoutBuffer) {
    static std::istringstream iss(input);
    std::cin.rdbuf(iss.rdbuf()); // Redirect cin to read from our input string
    originalCoutBuffer = std::cout.rdbuf(); // Store the original cout buffer
    std::cout.rdbuf(output.rdbuf()); // Redirect cout to write to our output string stream
}

// Helper function to restore the original cout buffer
void restoreCout(std::streambuf* originalCoutBuffer) {
    std::cout.rdbuf(originalCoutBuffer); // Restore the original cout buffer
}

TEST_CASE("Single word is added correctly", "[readInputWords]") {
    s_wordsMap.clear(); // Clear the map before each test

    setupCin("hello\nend\n");

    readInputWords();

    REQUIRE(s_wordsMap.size() == 1);
    REQUIRE(s_wordsMap["hello"] == 1);
}

TEST_CASE("Duplicate words are counted correctly", "[readInputWords]") {
    s_wordsMap.clear();

    setupCin("hello\nhello\nend\n");

    readInputWords();

    REQUIRE(s_wordsMap.size() == 1);
    REQUIRE(s_wordsMap["hello"] == 2);
}

TEST_CASE("Multiple words are added correctly", "[readInputWords]") {
    s_wordsMap.clear();

    setupCin("hello\nworld\nend\n");

    readInputWords();

    REQUIRE(s_wordsMap.size() == 2);
    REQUIRE(s_wordsMap["hello"] == 1);
    REQUIRE(s_wordsMap["world"] == 1);
}

TEST_CASE("Words with special characters are added correctly", "[readInputWords]") {
    s_wordsMap.clear();

    setupCin("hello!\nworld@\nend\n");

    readInputWords();

    REQUIRE(s_wordsMap.size() == 2);
    REQUIRE(s_wordsMap["hello!"] == 1);
    REQUIRE(s_wordsMap["world@"] == 1);
}

TEST_CASE("Empty lines are ignored", "[readInputWords]") {
    s_wordsMap.clear();

    setupCin("\n\nhello\n\n\nend\n");

    readInputWords();

    REQUIRE(s_wordsMap.size() == 1);
    REQUIRE(s_wordsMap["hello"] == 1);
}

TEST_CASE("Case sensitivity", "[readInputWords]") {
    s_wordsMap.clear();

    setupCin("hello\nHello\nHELLO\nend\n");

    readInputWords();

    REQUIRE(s_wordsMap.size() == 3);
    REQUIRE(s_wordsMap["hello"] == 1);
    REQUIRE(s_wordsMap["Hello"] == 1);
    REQUIRE(s_wordsMap["HELLO"] == 1);
}

TEST_CASE("EOF is handled gracefully", "[readInputWords]") {
    s_wordsMap.clear();

    const std::string simulatedEOF = ""; // Simulating EOF
    setupCin(simulatedEOF);

    readInputWords();

    // Check that the map is empty and no words were added
    REQUIRE(s_wordsMap.empty());
}

TEST_CASE("Random and non-visible characters are handled correctly", "[readInputWords]") {
    s_wordsMap.clear();

    // Create a string with a mix of visible, non-visible, and random characters
    std::string input = "hello\nworld\n\hello world\nx01\x02\x03\nrandom\x7F\xFF\n\033[1mworld\033[0m\n!@#$%^&*\n\nend\n";

    setupCin(input);

    readInputWords();

    // Check that the words are added correctly, ignoring non-visible characters
    REQUIRE(s_wordsMap.size() == 3);
    REQUIRE(s_wordsMap["hello"] == 1);
    REQUIRE(s_wordsMap["world"] == 1);
    REQUIRE(s_wordsMap["hello world"] == 0); // Multiple words per line aren't allowed
    REQUIRE(s_wordsMap["random"] == 0); // Non-visible characters aren't allowed
    REQUIRE(s_wordsMap["\033[1mworld\033[0m"] == 0); // Non-visible (ANSI escape codes for bold) aren't allowed
    REQUIRE(s_wordsMap["!@#$%^&*"] == 1); // Special characters are allowed
}

TEST_CASE("lookupWords finds and reports words correctly", "[lookupWords]") {
    s_wordsMap.clear();
    s_totalFound = 0; // Reset the total found count

    // Add some words to the map
    s_wordsMap["hello"] = 2;
    s_wordsMap["world"] = 1;

    // Simulate user input for lookup and EOF
    std::ostringstream output;
    std::streambuf* originalCoutBuffer;
    setupIo("hello\nworld\nnotfound\n", output, originalCoutBuffer);

    lookupWords();

    // Restore the original cout buffer to not break catch2
    restoreCout(originalCoutBuffer);

    // Check the output and the total found count
    // Note, normally there would be a newline after each line 
    // "Enter a word for lookup:", but because we are injecting
    // cin via a stream you won't see it in the output.
    std::string expectedOutput = "\n"
        "Enter a word for lookup:"
        "SUCCESS: 'hello' was present 2 times in the initial word list\n"
        "\n"
        "Enter a word for lookup:"
        "SUCCESS: 'world' was present 1 times in the initial word list\n"
        "\n"
        "Enter a word for lookup:"
        "'notfound' was NOT found in the initial word list\n";
    
    std::cout << output.str();
    REQUIRE(output.str().compare(expectedOutput));
    REQUIRE(s_totalFound == 2);
}