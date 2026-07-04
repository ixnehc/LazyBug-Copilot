# Third-Party Licenses

This project (LazyBug-Copilot) uses the following third-party components, each distributed under its own license.

---

## LLVM / Clang

- **Component:** libclang (LLVM/Clang)
- **License:** Apache License 2.0 with LLVM Exceptions
- **Website:** https://llvm.org/
- **Usage:** Used for C/C++ code parsing and symbol extraction.

```
==============================================================================
The LLVM Project is under the Apache License v2.0 with LLVM Exceptions:
==============================================================================

                                 Apache License
                           Version 2.0, January 2004
                        http://www.apache.org/licenses/

    TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

    1. Definitions.

      "License" shall mean the terms and conditions for use, reproduction,
      and distribution as defined by Sections 1 through 9 of this document.

      ... (full text at https://www.apache.org/licenses/LICENSE-2.0)

==============================================================================
---- LLVM Exceptions to the Apache 2.0 License ----
==============================================================================

As an exception, if, as a result of your compiling your source code, portions
of this Software are embedded into an Object form of such source code, you
may redistribute such embedded portions in such Object form without complying
with the conditions of Sections 4(a), 4(b) and 4(d) of the License.
```

> Full license text: https://llvm.org/LICENSE.txt

---

## tree-sitter

- **Component:** tree-sitter runtime library
- **License:** MIT
- **Website:** https://tree-sitter.github.io/
- **Usage:** Used as the incremental parsing framework for syntax analysis.

```
Copyright (c) 2018 Max Brunsfeld

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

> Full license: https://github.com/tree-sitter/tree-sitter/blob/master/LICENSE

---

## tree-sitter Grammars (Parser Files)

This project includes grammar scanner/parser source files generated from or derived from the following tree-sitter grammar repositories, all under the **MIT License**:

| Language   | Source Repository |
|------------|-------------------|
| C          | https://github.com/tree-sitter/tree-sitter-c          |
| C++        | https://github.com/tree-sitter/tree-sitter-cpp        |
| C#         | https://github.com/tree-sitter/tree-sitter-c-sharp    |
| Python     | https://github.com/tree-sitter/tree-sitter-python     |
| JavaScript | https://github.com/tree-sitter/tree-sitter-javascript |
| TypeScript | https://github.com/tree-sitter/tree-sitter-typescript |
| CSS        | https://github.com/tree-sitter/tree-sitter-css        |
| HTML       | https://github.com/tree-sitter/tree-sitter-html       |

```
MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
```

---

## Lucene++

- **Component:** Lucene++ (C++ port of Apache Lucene)
- **License:** Apache License 2.0 / LGPL v3 (dual-licensed)
- **Website:** https://github.com/luceneplusplus/LucenePlusPlus
- **Usage:** Used for full-text search and indexing functionality.
- **License files:** [APACHE.license](https://raw.githubusercontent.com/luceneplusplus/LucenePlusPlus/master/APACHE.license), [COPYING](https://raw.githubusercontent.com/luceneplusplus/LucenePlusPlus/master/COPYING), [LGPL.license](https://raw.githubusercontent.com/luceneplusplus/LucenePlusPlus/master/LGPL.license)

```
                                 Apache License
                           Version 2.0, January 2004
                        http://www.apache.org/licenses/

   ... (full text at https://www.apache.org/licenses/LICENSE-2.0)
```

---

## curl / libcurl

- **Component:** libcurl
- **License:** curl License (MIT-like)
- **Website:** https://curl.se/
- **Usage:** Used for HTTP/HTTPS network requests.

```
COPYRIGHT AND PERMISSION NOTICE

Copyright (c) 1996 - 2026, Daniel Stenberg, <daniel@haxx.se>, and many
contributors, see the THANKS file.

All rights reserved.

Permission to use, copy, modify, and distribute this software for any purpose
with or without fee is hereby granted, provided that the above copyright
notice and this permission notice appear in all copies.
```

> Full license: https://curl.se/docs/copyright.html

---

## nlohmann/json

- **Component:** JSON for Modern C++
- **License:** MIT
- **Website:** https://github.com/nlohmann/json
- **Usage:** Used for JSON serialization and parsing.

```
MIT License

Copyright (c) 2013-2026 Niels Lohmann

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
```

> Full license: https://github.com/nlohmann/json/blob/develop/LICENSE.MIT

---

## Boost C++ Libraries

- **Component:** Boost C++ Libraries
- **License:** Boost Software License 1.0
- **Website:** https://www.boost.org/
- **Usage:** Used for various general-purpose C++ utilities (filesystem, regex, containers, etc.).

```
Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
```

> Full license: https://www.boost.org/LICENSE_1_0.txt

---

## myers-diff

- **Component:** myers-diff (based on diff-match-patch)
- **License:** Apache License 2.0
- **Source:** https://github.com/gritzko/myers-diff
- **Usage:** Used for computing text diffs via Myers' algorithm. Included in `Common/CodeDiff/`.

```
Copyright 2018 The diff-match-patch Authors.
Copyright 2019 Victor Grishchenko

Licensed under the Apache License, Version 2.0
See Apache 2.0 full text above or at http://www.apache.org/licenses/LICENSE-2.0
```

---

## Summary Table

| Component        | License                | Link Type            |
|------------------|------------------------|----------------------|
| LLVM / libclang  | Apache 2.0 + Exception | Dynamic link (.dll)  |
| tree-sitter      | MIT                    | Static link (.lib)   |
| tree-sitter grammars | MIT                | Source inclusion     |
| Lucene++         | Apache 2.0 / LGPL 2.1  | Static link (.lib)   |
| curl / libcurl   | curl License           | Dynamic link (.dll)  |
| nlohmann/json    | MIT                    | Header-only          |
| Boost            | Boost Software 1.0     | Header-only          |
| myers-diff       | Apache 2.0             | Source inclusion     |
