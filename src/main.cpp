#include <iostream>
#include "SparqlReflector.hpp"
#include "NetworkClient.hpp"

// Define a C++ struct that represents the data we want from the Knowledge Graph
// The member names will automatically become SPARQL variables: ?item ?itemLabel
struct WikidataItem {
    std::string item;
    std::string itemLabel;
};

int main() {
    // 1. Reflect on the struct to build the query
    std::cout << "--- C++26 Reflection SPARQL Generator ---" << std::endl;
    
    // Logic: Find cats (Q146) on Wikidata
    std::string whereClause = "{ ?item wdt:P31 wd:Q146. ?item rdfs:label ?itemLabel. FILTER(LANG(?itemLabel) = 'en') }";
    
    // Generate the full query string using reflection
    std::string query = SparqlReflector::buildSimpleQuery<WikidataItem>(whereClause, 5);
    
    std::cout << "Generated Query: \n" << query << std::endl;

    // 2. Execute the query against Wikidata
    NetworkClient client;
    std::string endpoint = "https://query.wikidata.org/sparql";
    std::string fullUrl = endpoint + "?query=" + client.urlEncode(query);
    
    std::cout << "\n--- Executing Request to Wikidata ---" << std::endl;
    std::string response = client.performGet(fullUrl);

    // Output a snippet of the response
    std::cout << "Response (First 300 chars): " << std::endl;
    std::cout << response.substr(0, 300) << "..." << std::endl;

    return 0;
}