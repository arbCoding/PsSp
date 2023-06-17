# Development Tools

## Purpose
---

The purpose of this document is to provide a list of the tools that are being used in the development of PsSp as well as some information on why they are being used. This is an evolving document that will likely grow as I include more and more tools into the workflow.

## Compilers
---

1) [g++](https://gcc.gnu.org/) - This is the C++ compiler from the GNU Compiler Collection (GCC). I use it to compile on Linux (in particular, g++-12).
2) [clang++](https://clang.llvm.org/) - This is C++ compiler from the Low-Level Virtual Machine (LLVM) compiler infrastructure. I use it to compile on MacOS.

## Integrated Development Environment (IDE)
---

1) [VSCodium](https://vscodium.com/) - This is essentially [VSCode](https://code.visualstudio.com/), with the monitoring/telemetry from Microsoft removed.

### VSCodium Extensions

1) [C/C++ Advanced Lint](https://open-vsx.org/extension/jbenden/c-cpp-flylint) - This automatically runs the various [static analysis tools](#static-analysis) within VSCodium.
2) [CodeLLDB](https://open-vsx.org/extension/vadimcn/vscode-lldb) - Debugger that integrates with VSCodium.
3) [Git Extension Pack](https://open-vsx.org/extension/sugatoray/vscode-git-extension-pack) - Convenience package that provides git integration to VS Codium.

    a) [GitLens - Git supercharged](https://open-vsx.org/extension/eamodio/gitlens)

    b) [Git History](https://open-vsx.org/extension/donjayamanne/githistory)

    c) [Git Graph](https://open-vsx.org/vscode/item?itemName=mhutchie.git-graph)

    d) [Git File History](https://marketplace.visualstudio.com/items?itemName=pomber.git-file-history)

    e) [gitignore](https://open-vsx.org/extension/codezombiech/gitignore)

    f) [Git Blame](https://open-vsx.org/vscode/item?itemName=waderyan.gitblame)

    g) [Git History Diff](https://marketplace.visualstudio.com/items?itemName=huizhou.githd)

    h) [Commit Message Editor](https://open-vsx.org/extension/adam-bender/commit-message-editor)

    i) [Conentional Commits](https://open-vsx.org/vscode/item?itemName=vivaxy.vscode-conventional-commits)

    j) [GitHub Theme](https://open-vsx.org/extension/GitHub/github-vscode-theme)

    k) [Git Cheatsheet](https://marketplace.visualstudio.com/items?itemName=dzhavat.git-cheatsheet)

4) [Dark Github Markdown Pack](https://open-vsx.org/extension/sndst00m/markdown-github-dark-pack)

    a) [Dark Github Markdown Preview](https://marketplace.visualstudio.com/items?itemName=ozaki.markdown-github-dark)

    b) [Markdown Emoji](https://marketplace.visualstudio.com/items?itemName=bierner.markdown-emoji)

    c) [Markdown Checkboxes](https://marketplace.visualstudio.com/items?itemName=bierner.markdown-checkbox)

    d) [Markdown yaml Preamble](https://marketplace.visualstudio.com/items?itemName=bierner.markdown-yaml-preamble)

5) [clangd](https://open-vsx.org/extension/llvm-vs-code-extensions/vscode-clangd) - Provides auto-complete functionality for VSCodium (language server).

## Static Analysis
---

1) [clangd](https://clangd.llvm.org/)
2) [CppCheck](https://cppcheck.sourceforge.io/)
3) [Lizard](https://github.com/terryyin/lizard)
4) [FlawFinder](https://dwheeler.com/flawfinder/)

## Dynamic Analysis
---

## Testing
---

1) [Catch2](https://github.com/catchorg/Catch2)

## C++ References
---

1) [cppreference.com](https://en.cppreference.com/w/)
2) [isocpp.org](https://isocpp.org/)
3) [learncpp.com](https://www.learncpp.com/)
4) [r/cpp](https://www.reddit.com/r/cpp)
