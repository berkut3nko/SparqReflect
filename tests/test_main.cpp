#include <gtest/gtest.h>
#include "../src/SparqlReflector.hpp"

// Define test structs for reflection
struct Person {
    std::string name;
    int age;
};

struct Book {
    std::string title;
    std::string author;
    int year;
};

// Test 1: Verify SELECT clause generation for a simple struct
TEST(ReflectionTest, GenerateSelectPerson) {
    std::string result = SparqlReflector::generateSelectClause<Person>();
    
    // We expect the variable names to match member names
    // Note: The order depends on member declaration order
    std::string expected = "SELECT ?name ?age";
    EXPECT_EQ(result, expected);
}

// Test 2: Verify SELECT clause generation for a larger struct
TEST(ReflectionTest, GenerateSelectBook) {
    std::string result = SparqlReflector::generateSelectClause<Book>();
    std::string expected = "SELECT ?title ?author ?year";
    EXPECT_EQ(result, expected);
}

// Test 3: Verify full query construction
TEST(ReflectionTest, BuildFullQuery) {
    std::string where = "{ ?name rdf:type foaf:Person }";
    std::string result = SparqlReflector::buildSimpleQuery<Person>(where, 50);
    
    std::string expected = "SELECT ?name ?age WHERE { ?name rdf:type foaf:Person } LIMIT 50";
    EXPECT_EQ(result, expected);
}

// Entry point for tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}