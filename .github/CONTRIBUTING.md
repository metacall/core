# Contributing

When contributing to this repository, please first discuss the change you wish to make via issue,
email, or any other method with the owners of this repository before making a change. Here is an exemplary process you can follow to create an issue on MetaCall:

1. Identify a problem or a possible addition to Metacall codebase/operation<br>
for instance: ```pre-commit-clang-format not working on windows...```
2. Then go to the necessary MetaCall gitHub repository and click on the "Issues" tab at the top of the page. Indicate your issue's interest by tagging it as a bug report, custom issue, documentation, feature, or discussion. <br>
in this case: ```metacall/core```
3. Type in a title and description for your issue. Be as detailed as possible, including any error messages or steps to reproduce the issue. Image example: <br>
![issue image](https://user-images.githubusercontent.com/93955843/220493308-0ce3f101-6957-43fb-96d1-7a730ccf304c.PNG)

4. You can assign the issue to yourself, add labels or a milestone to it, and attach files if necessary.
5. Submit the newly created issue and start working on a solution by creating a fork of the repository. <br>(<a href="https://github.com/metacall/core/issues/370">Here</a> to issue used for explanation)

Please note we have a [Code of Conduct](/.github/CODE_OF_CONDUCT.md), please follow it in all your interactions with the project.

## Pull Request Process

1. **Fork and Branch:** Create a personal fork and branch off from `develop` (e.g., `git checkout -b fix/issue-name develop`). Do not target `master` for development PRs.
2. **Optional Good Practice:** Leave a comment on the issue indicating that you are working on it.
3. **Implement Changes:** Keep commits atomic, well-tested, and follow conventional commit message formats.
4. **Format Code:** Run `make clang-format` prior to committing to ensure all C/C++ files adhere to repository code styling.
5. **Test Locally:** Verify that all relevant tests pass (`ctest` / Docker tests) before pushing your changes.
6. **Update Documentation:** Include relevant documentation updates under `docs/` if your changes introduce new features or behavior changes.
7. **Submit PR:** Push your branch to your fork and submit a Pull Request targeting `metacall/core:develop`. Fill out the Pull Request template completely, disclosing any AI usage and linking the issue with `Fixes #<issue_no>`.

## Build Flows & Compilation

MetaCall uses modern CMake. Recommended build workflows:

### Standard Build Flow

```bash
# Configure the build directory
cmake -S . -B build

# Build and install locally
cmake --build build --target install
```

### Enabling Specific Loaders

You can toggle language loaders using CMake flags:

```bash
# Example: Building with Python and NodeJS loaders enabled
cmake -S . -B build -DOPTION_BUILD_LOADERS_PY=On -DOPTION_BUILD_LOADERS_NODE=On
cmake --build build
```

## Testing & Quality Checks

### Running Tests via CTest

To run a single test with verbose output:

```bash
ctest -VV -R metacall-python-test
```

To run the complete test suite:

```bash
ctest --output-on-failure
```

### Memory & Sanitizer Checks

Before submitting performance-critical or low-level changes:

- **Address Sanitizer (ASan):** Build with `-DOPTION_BUILD_ADDRESS_SANITIZER=On` or execute `./docker-compose.sh test-address-sanitizer &> output`.
- **Thread Sanitizer (TSan):** Build with `-DOPTION_BUILD_THREAD_SANITIZER=On` or execute `./docker-compose.sh test-thread-sanitizer &> output`.
- **Memory Check (Valgrind / Memcheck):** Run with `-DOPTION_TEST_MEMORYCHECK=On`.
- **Thread Race Detection (Helgrind):** Run Helgrind when modifying concurrent or multi-threaded runtime paths.

### Code Style & Formatting

- **Format Command:** Run `make clang-format` in order to format your code to match style guidelines.

## Coding Style Guidelines

- **Standards:** Default to **C23** for C code and **C++23** for C++ code where applicable.
- **Compiler Flags:** Code must compile cleanly with strict compiler flags:
  `-Wall -Wextra -Werror -fsanitize=address,undefined`
- **Cleanliness:** No dead code, debug prints, or unreferenced variables should remain in submitted PRs.

## Project Resources & Policies

- **Security Policy:** Refer to [SECURITY.md](../SECURITY.md) to report vulnerabilities privately.
- **Code of Conduct:** Review our [CODE_OF_CONDUCT.md](../CODE_OF_CONDUCT.md) for community standards and reporting guidelines.
- **Roadmap:** View the [MetaCall Roadmap](../ROADMAP.md) for upcoming milestones and plans.
- **Support Channels:** Check [SUPPORT.md](../SUPPORT.md) or join our [Discord Server](https://discord.com/invite/upwP4mwJWa) for community help.