#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <format>
#include <string_view>
#include <utility>

// P2996 Standard Header
#include <meta> 

namespace meta = std::meta;

namespace reflection_impl {
    // Helper for expansion statements workaround (P2996R13 Section 2.3)
    // Allows iterating over ranges that allocate memory (like std::vector from members_of)
    // by converting them into a template parameter pack.
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
 * @brief  A utility class to generate SPARQL query parts using C++26 Static Reflection.
 */
class SparqlReflector {
public:
    // Helper function to expand a range into a splice-able replicator.
    // This consumes the vector immediately in a consteval context, avoiding
    // persistent heap allocation issues in 'template for'.
    template<typename R>
    static consteval auto expand(R range) {
        std::vector<std::meta::info> args;
        for (auto r : range) {
            args.push_back(std::meta::reflect_constant(r));
        }
        return meta::substitute(^^reflection_impl::replicator, args);
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

        // P2996 Reflection
        using ReflectedType = T;
        constexpr auto type_meta = ^^ReflectedType;

        // FIX for P2996R13 / Clang implementation limitations:
        // Direct use of 'template for' with 'members_of' fails because the returned vector
        // cannot be held in a constexpr variable due to heap allocation.
        // We use the 'expand' pattern to unpack members into a template lambda.
        // P2996R10+: members_of requires an access_context argument.
        [:expand(meta::members_of(type_meta, meta::access_context::unchecked())):] >> [&]<auto member>{
            
            // Filter: We only want non-static data members (fields)
            if constexpr (meta::is_nonstatic_data_member(member)) {
                // Extract the identifier (name) of the member.
                constexpr auto name = meta::identifier_of(member);
                
                // Append to the query string with a question mark prefix
                ss << " ?" << name;
            }
        };

        return ss.str();
    }

    /**
     * @brief construct a simple query wrapper.
     * @tparam T The struct defining the expected result columns.
     * @param whereClause The raw SPARQL WHERE clause.
     * @param limit The maximum number of results (default 10).
     * @return std::string Full executable SPARQL query.
     */
    template <typename T>
    static std::string buildSimpleQuery(std::string_view whereClause, int limit = 10) {
        std::string select = generateSelectClause<T>();
        return std::format("{} WHERE {} LIMIT {}", select, whereClause, limit);
    }
};