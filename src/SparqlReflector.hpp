#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <format>
#include <string_view>
#include <utility>
#include <iostream>
#include <algorithm>
#include <type_traits> // Required for std::is_integral_v, etc.
#include <print>       // C++23: Modern printing

// P2996 Standard Header
#include <meta> 

namespace meta = std::meta;

namespace reflection_impl {
    // Helper for expansion statements workaround (P2996R13 Section 2.3)
    template<auto... vals>
    struct replicator_type {
        template<typename F>
        constexpr void operator>>(F body) const {
            (body.template operator()<vals>(), ...);
        }
    };

    template<auto... vals>
    replicator_type<vals...> replicator = {};
}

/**
 * @brief Simple helper class to parse SPARQL JSON responses without external libraries.
 * Designed specifically for the structure: { "results": { "bindings": [ ... ] } }
 */
class MiniSparqlParser {
public:
    /**
     * @brief Extract individual binding objects (rows) from the full JSON response.
     * @param json Full JSON string.
     * @return std::vector<std::string> List of JSON substrings, each representing one row item.
     */
    static std::vector<std::string> extractBindings(std::string_view json) {
        std::vector<std::string> bindings;
        
        // 1. Find "results"
        size_t resultsPos = json.find("\"results\"");
        if (resultsPos == std::string_view::npos) return bindings;

        // 2. Find "bindings" inside results
        size_t bindingsPos = json.find("\"bindings\"", resultsPos);
        if (bindingsPos == std::string_view::npos) return bindings;

        // 3. Find the start of the array '['
        size_t arrayStart = json.find('[', bindingsPos);
        if (arrayStart == std::string_view::npos) return bindings;

        // 4. Iterate through array elements (objects enclosed in { })
        size_t currentPos = arrayStart + 1;
        int braceCount = 0;
        size_t itemStart = std::string_view::npos;

        while (currentPos < json.length()) {
            char c = json[currentPos];
            if (c == '{') {
                if (braceCount == 0) itemStart = currentPos;
                braceCount++;
            } else if (c == '}') {
                braceCount--;
                if (braceCount == 0 && itemStart != std::string_view::npos) {
                    // Found a complete object { ... }
                    bindings.emplace_back(json.substr(itemStart, currentPos - itemStart + 1));
                    itemStart = std::string_view::npos;
                }
            } else if (c == ']' && braceCount == 0) {
                break; // End of bindings array
            }
            currentPos++;
        }
        return bindings;
    }

    /**
     * @brief Extracts the "value" of a specific variable from a binding JSON string.
     * Looks for: "varName": { ... "value": "TARGET_VALUE" ... }
     * Robust to whitespace.
     * @param rowJson The JSON string for a single row.
     * @param varName The variable name to search for (e.g., "itemLabel").
     * @return std::string The extracted value or empty string if not found.
     */
    static std::string extractValue(std::string_view rowJson, std::string_view varName) {
        // Construct search key like "varName"
        std::string key = "\"" + std::string(varName) + "\"";
        
        size_t keyPos = rowJson.find(key);
        if (keyPos == std::string_view::npos) return "";

        // Find the opening brace of this variable's object '{'
        // Skip whitespace after key
        size_t objStart = rowJson.find('{', keyPos);
        if (objStart == std::string_view::npos) return "";

        // Inside this object, find "value" key
        size_t valueLabelPos = rowJson.find("\"value\"", objStart);
        if (valueLabelPos == std::string_view::npos) return "";

        // Find the colon after "value"
        size_t colonPos = rowJson.find(':', valueLabelPos);
        if (colonPos == std::string_view::npos) return "";

        // Find the opening quote of the actual value, skipping whitespace/newlines
        size_t valueStartQuote = std::string_view::npos;
        for (size_t i = colonPos + 1; i < rowJson.length(); ++i) {
            if (rowJson[i] == '"') {
                valueStartQuote = i;
                break;
            }
        }
        if (valueStartQuote == std::string_view::npos) return "";

        // Find the closing quote (handling escaped quotes is skipped for simplicity)
        size_t valueEndQuote = rowJson.find('"', valueStartQuote + 1);
        if (valueEndQuote == std::string_view::npos) return "";

        return std::string(rowJson.substr(valueStartQuote + 1, valueEndQuote - valueStartQuote - 1));
    }
};

/**
 * @brief  A utility class to generate SPARQL query parts and parse results using C++26 Static Reflection.
 */
class SparqlReflector {
public:
    // Helper function to expand a range into a splice-able replicator.
    template<typename R>
    static consteval auto expand(R range) {
        std::vector<std::meta::info> args;
        for (auto r : range) {
            args.push_back(std::meta::reflect_constant(r));
        }
        return meta::substitute(^^reflection_impl::replicator, args);
    }

    /**
     * @brief Automatically prints any C++ struct to the console using Reflection.
     * @tparam T Any struct with members.
     * @param obj The object to print.
     */
    template <typename T>
    static void printStruct(const T& obj) {
        constexpr auto type_meta = ^^T;
        
        std::print("  [ ");
        
        bool is_first = true;
        
        // Iterate over all members of the struct
        [:expand(meta::members_of(type_meta, meta::access_context::unchecked())):] >> [&]<auto member>{
            if constexpr (meta::is_nonstatic_data_member(member)) {
                if (!is_first) std::print(" | ");
                
                // Print "MemberName: Value"
                std::print("{}: {}", meta::identifier_of(member), obj.[:member:]);
                is_first = false;
            }
        };
        
        std::println(" ]");
    }

    /**
     * @brief Generates a SPARQL SELECT clause string from a struct type.
     * @tparam T The struct type to reflect upon.
     * @return std::string A string formatted as "SELECT ?member1 ?member2 ..."
     */
    template <typename T>
    static std::string generateSelectClause() {
        std::stringstream ss;
        ss << "SELECT";

        using ReflectedType = T;
        constexpr auto type_meta = ^^ReflectedType;

        // P2996R10+: members_of requires an access_context argument.
        [:expand(meta::members_of(type_meta, meta::access_context::unchecked())):] >> [&]<auto member>{
            if constexpr (meta::is_nonstatic_data_member(member)) {
                constexpr auto name = meta::identifier_of(member);
                ss << " ?" << name;
            }
        };

        return ss.str();
    }

    /**
     * @brief Construct a simple query wrapper.
     * @tparam T The struct defining the expected result columns.
     */
    template <typename T>
    static std::string buildSimpleQuery(std::string_view whereClause, int limit = 10) {
        std::string select = generateSelectClause<T>();
        return std::format("{} WHERE {} LIMIT {}", select, whereClause, limit);
    }

    /**
     * @brief Parses a SPARQL JSON response into a vector of struct T using Reflection and a custom mini-parser.
     * @tparam T The target struct type to populate.
     * @param rawJson The raw JSON string returned by the SPARQL endpoint.
     * @return std::vector<T> A list of populated objects.
     */
    template <typename T>
    static std::vector<T> parseJsonResponse(const std::string& rawJson) {
        std::vector<T> results;
        
        // 1. Extract raw JSON strings for each row
        auto rows = MiniSparqlParser::extractBindings(rawJson);

        // 2. Iterate over rows and map to C++ struct using Reflection
        for (const auto& rowJson : rows) {
            T item{}; // Value initialization (important for ints to be 0)
            using ReflectedType = T;
            constexpr auto type_meta = ^^ReflectedType;

            // Reflection Loop: Iterate over struct members
            [:expand(meta::members_of(type_meta, meta::access_context::unchecked())):] >> [&]<auto member>{
                if constexpr (meta::is_nonstatic_data_member(member)) {
                    // Get compile-time name of the member (e.g., "itemLabel")
                    constexpr auto nameView = meta::identifier_of(member);
                    
                    // Extract value from JSON using our mini-parser
                    std::string value = MiniSparqlParser::extractValue(rowJson, nameView);
                    
                    if (!value.empty()) {
                        // Get the type of the member to perform correct conversion
                        using MemberType = typename [: meta::type_of(member) :];

                        // Assign based on type
                        if constexpr (std::is_same_v<MemberType, std::string>) {
                            item.[:member:] = value;
                        } 
                        else if constexpr (std::is_integral_v<MemberType>) {
                            // Convert string to integer type (int, long, size_t, etc.)
                            try {
                                item.[:member:] = static_cast<MemberType>(std::stoll(value));
                            } catch (...) {
                                // Ignore conversion errors in this simple parser
                            }
                        }
                        else if constexpr (std::is_floating_point_v<MemberType>) {
                            // Convert string to floating point (float, double)
                            try {
                                item.[:member:] = static_cast<MemberType>(std::stod(value));
                            } catch (...) {
                                // Ignore errors
                            }
                        }
                    }
                }
            };
            
            results.push_back(item);
        }
        return results;
    }
};