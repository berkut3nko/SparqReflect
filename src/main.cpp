#include <print> // New C++23 feature for clean output
#include "SparqlReflector.hpp"
#include "NetworkClient.hpp"

// Our Data Model
// We just define what we want, and C++ Reflection handles the rest.
struct WikidataItem {
    std::string item;
    std::string itemLabel;
};

int main() {
    std::println("--- C++26 SPARQL Query Demo ---");

    // 1. Define WHAT we are looking for (Logic)
    // "Find cats (Q146) and their labels in English"
    std::string logic = R"(
        { 
            ?item wdt:P31 wd:Q146. 
            ?item rdfs:label ?itemLabel. 
            FILTER(LANG(?itemLabel) = 'en') 
        }
    )";
    
    // 2. Build the Query automatically
    // The Reflector looks at 'WikidataItem' struct and generates: "SELECT ?item ?itemLabel ..."
    std::string query = SparqlReflector::buildSimpleQuery<WikidataItem>(logic, 3);
    
    std::println("Generated Query:\n{}", query);

    // 3. Send Request to Wikidata
    std::println("\n--- Sending Request... ---");
    
    NetworkClient client;
    std::string url = "https://query.wikidata.org/sparql?query=" + client.urlEncode(query);
    std::string json = client.performGet(url);

    if (json.empty()) {
        std::println(stderr, "Error: No response from Wikidata.");
        return 1;
    }

    // 4. Convert JSON response to C++ objects
    std::println("\n--- Processing Results... ---");
    auto results = SparqlReflector::parseJsonResponse<WikidataItem>(json);

    std::println("Found {} results:", results.size());

    // 5. Print results using Reflection
    // We don't need to manually type "item.label", the printer figures it out.
    for (const auto& item : results) {
        SparqlReflector::printStruct(item);
    }

    return 0;
}