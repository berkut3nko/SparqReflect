![Standard](https://img.shields.io/badge/Standard-C%2B%2B%2026-blue?logo=c%2B%2B&logoColor=white) ![Reflection](https://img.shields.io/badge/Reflection-P2996-orange) ![Compiler](https://img.shields.io/badge/Compiler-Clang%20(Experimental)-red?logo=llvm&logoColor=white) ![Build](https://img.shields.io/badge/Build-CMake-008FBA?logo=cmake&logoColor=white) ![License](https://img.shields.io/badge/License-MIT-yellow.svg)
# SparqReflect: AI Knowledge Mapping with C++26

**SparqReflect** is a research project demonstrating the use of upcoming C++26 Static Reflection (P2996) features to efficiently parse and map Semantic Web data (RDF/SPARQL) from public endpoints like Wikidata.

The project leverages libcurl for network requests and maps semantic graph data directly to C++ structs using zero-overhead compile-time reflection.

## 🚀 Features

**Modern C++26:** Utilizes the experimental P2996 proposal for Static Reflection.

**Semantic Web:** Interacts with Wikidata SPARQL endpoints.

**Networking:** Embedded libcurl via CMake FetchContent.

**Testing:** Unit testing with GoogleTest.

## 🛠️ Prerequisites

To compile the Reflection features, you cannot use a standard install of GCC or Clang (as of early 2026). You must use a compiler fork that implements P2996.

### Recommended Compiler

**Clang P2996 Fork:** 
```bash
Bloomberg Clang-p2996
```

**Compiler Explorer:** You can test the code logic online at godbolt.org by selecting the x86-64 clang (p2996) compiler.

### Dependencies

- CMake 3.25+

- OpenSSL (recommended for HTTPS support in cURL)

**Linux:** ```sudo apt-get install libssl-dev```

**macOS:** ```brew install openssl```

## 🏗️ Build Instructions

1. Clone the repository:

```bash
git clone [https://github.com/your-username/SparqReflect.git](https://github.com/your-username/SparqReflect.git)
cd SparqReflect
```

2. Configure CMake:
`Note: You must point CMake to your specific P2996 compiler.`

```bash
mkdir build
cd build
cmake -DCMAKE_CXX_COMPILER=/path/to/clang-p2996/bin/clang++ ..
```


3. Build:
```bash
cmake --build .
```

4. Run:
```bash
./SparqReflect
```

5. Run Tests:
```bash
ctest --output-on-failure
```

## 📝 Project Structure

```text
SparqReflect/
├── CMakeLists.txt          # Build configuration (CMake)
├── Doxyfile                # Doxygen documentation config
├── README.md               # Project documentation
├── src/
│   ├── main.cpp            # Entry point (Usage example)
│   ├── NetworkClient.hpp   # HTTP Client wrapper (libcurl)
│   └── SPARQReflector.hpp # Core Reflection & parsing logic (P2996)
└── tests/
    └── test_main.cpp       # Unit tests (GoogleTest)
```

## ⚠️ Disclaimer

This project uses experimental C++ features that are not yet part of the official ISO standard. Syntax and behavior are subject to change.