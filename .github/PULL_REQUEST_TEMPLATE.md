# Description

Please include a summary of the change and which issue is fixed. List any dependencies that are required for this change.

Fixes #(issue_no)

<!-- Replace `issue_no` with the issue number which is fixed in this PR -->

## Type of change

<!-- Please delete options that are not relevant. -->

- [ ] Bug fix (non-breaking change which fixes an issue)
- [ ] New feature (non-breaking change which adds functionality)
- [ ] Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] This change requires a documentation update
- [ ] Documentation update

# Checklist:

- [ ] I have performed a self-review of my own code.
- [ ] I have commented my code, particularly in hard-to-understand areas.
- [ ] I have made corresponding changes to the documentation.
- [ ] My changes generate no new warnings.
- [ ] I have added tests/screenshots (if any) that prove my fix is effective or that my feature works.
- [ ] I have tested the tests implicated (if any) by my own code and they pass (`make test` or `ctest -VV -R <test-name>`).
- [ ] If my change is significant or breaking, I have passed all tests with `./docker-compose.sh build &> output` and attached the output.
- [ ] I have tested my code with `OPTION_BUILD_SANITIZER` or `./docker-compose.sh test &> output` and `OPTION_TEST_MEMORYCHECK`. 
- [ ] I have tested with `Helgrind` in case my code works with threading.
- [ ] I have run `make clang-format` in order to format my code and my code follows the style guidelines.

If you are unclear about any of the above checks, have a look at our documentation [here](https://github.com/metacall/core/blob/develop/docs/README.md#63-debugging).
