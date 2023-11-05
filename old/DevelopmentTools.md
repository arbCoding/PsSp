---
Author: "Alexander R. Blanchette"
Email: "ARBCoding@gmail.com"
---

# Development Tools

## Purpose

The purpose of this document is to provide a list of the tools that are being used in the development of PsSp as well as some information on why they are being used. This is an evolving document that will likely grow as I include more and more tools into the workflow.

## Compilers

1) [g++](https://gcc.gnu.org/) - This is the C++ compiler from the GNU Compiler Collection (GCC). I use it to compile on Linux (in particular, g++-12).
2) [clang++](https://clang.llvm.org/) - This is C++ compiler from the Low-Level Virtual Machine (LLVM) compiler infrastructure. I use it to compile on MacOS.

## Integrated Development Environment (IDE)

1) [VSCodium](https://vscodium.com/) - This is essentially [VSCode](https://code.visualstudio.com/), with the monitoring/telemetry from Microsoft removed.

### VSCodium Extensions

1) [C/C++ Advanced Lint](https://open-vsx.org/extension/jbenden/c-cpp-flylint) - This automatically runs most of the various [static analysis tools](#static-analysis) within VSCodium.
2) [CodeLLDB](https://open-vsx.org/extension/vadimcn/vscode-lldb) - Debugger that integrates with VSCodium.
3) [Git Extension Pack](https://open-vsx.org/extension/sugatoray/vscode-git-extension-pack) - Convenience package that provides git integration to VS Codium.
4) [Dark Github Markdown Pack](https://open-vsx.org/extension/sndst00m/markdown-github-dark-pack)
5) [clangd](https://open-vsx.org/extension/llvm-vs-code-extensions/vscode-clangd) - Provides auto-complete functionality for VSCodium (language server).
6) [SonarLint](https://open-vsx.org/extension/SonarSource/sonarlint-vscode) - Additional static analysis tool.
7) [Markdown Extension Pack](https://open-vsx.org/vscode/item?itemName=sugatoray.vscode-markdown-extension-pack)
8) [LaTeX Workshop](https://open-vsx.org/vscode/item?itemName=James-Yu.latex-workshop) - LaTeX typesetting efficiency with preview, compile, autocomplete, colorize, etc.
9) [LTeX – LanguageTool grammar/spell checking](https://open-vsx.org/vscode/item?itemName=valentjn.vscode-ltex)

## Static Analysis

1) [clangd](https://clangd.llvm.org/)
2) [CppCheck](https://cppcheck.sourceforge.io/)
3) [FlawFinder](https://dwheeler.com/flawfinder/)
4) [SonarLint](https://docs.sonarcloud.io/improving/sonarlint/)

## Dynamic Analysis

1) [Thread Sanitizer](https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual) - Check for data races
2) [Address Sanitizer](https://github.com/google/sanitizers/wiki/AddressSanitizer) - Check for memory errors
3) [Memory Sanitizer](https://github.com/google/sanitizers/wiki/MemorySanitizer) - Check for memory errors

## Testing

1) [Catch2](https://github.com/catchorg/Catch2)

## Miscellaneous

1) [Bear](https://github.com/rizsotto/Bear) - Used to generate compile_commands.json on Linux/MacOS
2) [compilelib](https://github.com/nickdiego/compiledb) - Used to generate compile_commands.json on Windows (Not working...)

## C++ References

1) [cppreference.com](https://en.cppreference.com/w/)
2) [isocpp.org](https://isocpp.org/)
3) [learncpp.com](https://www.learncpp.com/)
4) [r/cpp](https://www.reddit.com/r/cpp)
5) [C++20 Compiler Support](https://en.cppreference.com/w/cpp/compiler_support/20)
6) [LibC++20 Status](https://libcxx.llvm.org/Status/Cxx20.html)
7) [Incomplete List of Static Analysis Tools](https://github.com/analysis-tools-dev/static-analysis#cpp)
8) [Incomplete List of Dynamic Analysis Tools](https://github.com/analysis-tools-dev/dynamic-analysis#cpp)
9) [Making portable executables for Windows](https://stackoverflow.com/questions/31449769/distribute-a-program-compiled-with-mingw-g)

## Other References

1) [Latex documentation](https://en.wikibooks.org/wiki/LaTeX)
2) [Latex mathtools documentation](https://mirror.mwt.me/ctan/macros/latex/contrib/mathtools/mathtools.pdf)
