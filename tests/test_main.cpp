#include <gtest/gtest.h>
#include <string>
#include <vector>

// Include the header directly as CMake handles include directories
#include "SPARQReflector.hpp"

// =============================================================================
// Test Data Structures
// =============================================================================

struct Person {
    std::string name;
    int age;
};

struct Book {
    std::string title;
    std::string author;
    int year;
};

// =============================================================================
// Reflection Tests
// =============================================================================

/**
 * @brief Verify that SELECT clauses are generated correctly based on struct members.
 * The order of variables must match the declaration order in the struct.
 */
TEST(ReflectionTest, GenerateSelectClause) {
    // Case 1: Person struct
    {
        std::string result = SparqlReflector::generateSelectClause<Person>();
        std::string expected = "SELECT ?name ?age";
        EXPECT_EQ(result, expected) << "Failed to generate SELECT for Person";
    }

    // Case 2: Book struct
    {
        std::string result = SparqlReflector::generateSelectClause<Book>();
        std::string expected = "SELECT ?title ?author ?year";
        EXPECT_EQ(result, expected) << "Failed to generate SELECT for Book";
    }
}

/**
 * @brief Verify the construction of the full SPARQL query string.
 * This checks if the SELECT clause, WHERE clause, and LIMIT are combined correctly.
 */
TEST(ReflectionTest, BuildFullQuery) {
    std::string whereClause = "{ ?name rdf:type foaf:Person }";
    int limit = 50;

    std::string result = SparqlReflector::buildSimpleQuery<Person>(whereClause, limit);
    std::string expected = "SELECT ?name ?age WHERE { ?name rdf:type foaf:Person } LIMIT 50";
    
    EXPECT_EQ(result, expected);
}

/**
 * @brief Verify parsing logic using the MiniSparqlParser directly.
 * We simulate a raw JSON response to test extraction without network calls.
 */
TEST(ReflectionTest, ParseJsonResponse) {
    // Simulated JSON response from Wikidata
    // Note: The simple parser relies on finding "value" and extracting the string inside quotes.
    std::string rawJson = R"({
        "head": { "vars": [ "name", "age" ] },
        "results": {
            "bindings": [
                {
                    "name": { "type": "literal", "value": "Alice" },
                    "age": { "type": "literal", "value": "30" }
                },
                {
                    "name": { "type": "literal", "value": "Bob" },
                    "age": { "type": "literal", "value": "25" }
                }
            ]
        }
    })";

    std::vector<Person> people = SparqlReflector::parseJsonResponse<Person>(rawJson);

    ASSERT_EQ(people.size(), 2);
    
    EXPECT_EQ(people[0].name, "Alice");
    EXPECT_EQ(people[0].age, 30); 
    
    EXPECT_EQ(people[1].name, "Bob");
    EXPECT_EQ(people[1].age, 25);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}