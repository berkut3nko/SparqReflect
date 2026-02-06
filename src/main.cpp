#include <print>
#include <string>
#include <vector>
#include "SPARQReflector.hpp"

// =============================================================================
// DATA MODELS (Mapped via Reflection)
// =============================================================================

struct ProgrammingLanguage {
    std::string langLabel;
    std::string creatorLabel;
};

struct SpaceTelescope {
    std::string telescopeLabel;
    std::string launchDate;
};

struct RiverInfo {
    std::string riverLabel;
    double length;
};

// For Task 4: Aggregation
struct AstronautStats {
    std::string countryLabel;
    int count; // Maps to (?count) in SPARQL
};

// =============================================================================
// MAIN ENTRY POINT
// =============================================================================

int main() {
    std::println("--- SparqReflect: C++26 Semantic Web Client ---");

    // ---------------------------------------------------------
    // TASK 3: Basic Queries (Automatic Generation)
    // ---------------------------------------------------------

    // Scenario 1: Programming Languages
    std::string where1 = R"(
        {
            ?lang wdt:P31 wd:Q9143.
            ?lang wdt:P170 ?creator.
            ?lang rdfs:label ?langLabel.
            ?creator rdfs:label ?creatorLabel.
            FILTER(LANG(?langLabel) = 'en' && LANG(?creatorLabel) = 'en')
        }
    )";
    SparqlReflector::executeSimpleQueryScenario<ProgrammingLanguage>("Basic Query 1: Programming Languages", where1, 5);

    // Scenario 2: Space Telescopes
    std::string where2 = R"(
        {
            ?telescope wdt:P31 wd:Q35221.
            ?telescope wdt:P619 ?launchDate.
            ?telescope rdfs:label ?telescopeLabel.
            FILTER(LANG(?telescopeLabel) = 'en')
        }
    )";
    SparqlReflector::executeSimpleQueryScenario<SpaceTelescope>("Basic Query 2: Space Telescopes", where2, 5);

    // Scenario 3: Rivers (Optional Data)
    std::string where3 = R"(
        {
            ?river wdt:P31 wd:Q4022.
            ?river rdfs:label ?riverLabel.
            OPTIONAL { ?river wdt:P2043 ?length. }
            FILTER(LANG(?riverLabel) = 'en')
        }
    )";
    SparqlReflector::executeSimpleQueryScenario<RiverInfo>("Feature Query: Rivers (Optional Length)", where3, 5);

    // ---------------------------------------------------------
    // TASK 4: Advanced Query (Grouping & Ranking)
    // ---------------------------------------------------------
    
    std::string query4 = R"(
        SELECT ?countryLabel (COUNT(?astronaut) as ?count)
        WHERE {
            ?astronaut wdt:P106 wd:Q11631.  # Occupation: Astronaut
            ?astronaut wdt:P27 ?country.    # Country of citizenship
            ?country rdfs:label ?countryLabel.
            FILTER(LANG(?countryLabel) = 'en')
        }
        GROUP BY ?countryLabel
        ORDER BY DESC(?count)
        LIMIT 10
    )";

    SparqlReflector::executeRawQueryScenario<AstronautStats>("Advanced Query: Astronauts per Country", query4);

    return 0;
}