//g++ -std=c++23 -Wall -Wextra -Wpedantic 12.02.cpp -o 12.02.out
#include <cstdio>
#include <cassert>
#include <cstring>

static constexpr int ASCII_BACKSLASH = 92;
static constexpr int ASCII_DQUOTE = 34;

static const char *L[] = {
    "//g++ -std=c++23 -Wall -Wextra -Wpedantic 12.02.cpp -o 12.02.out",
    "#include <cstdio>",
    "#include <cassert>",
    "#include <cstring>",
    "",
    "static constexpr int ASCII_BACKSLASH = 92;",
    "static constexpr int ASCII_DQUOTE = 34;",
    "",
    "static const char *L[] = {",
    nullptr,
    "};",
    "",
    "static constexpr int LINE_COUNT = static_cast<int>(sizeof(L) / sizeof(L[0]));",
    "",
    "static void print_escaped(const char *s) {",
    "    for (int i = 0; s[i] != 0; ++i) {",
    "        if (s[i] == ASCII_BACKSLASH) {",
    "            std::printf(\"%c%c\", ASCII_BACKSLASH, ASCII_BACKSLASH);",
    "        } else if (s[i] == ASCII_DQUOTE) {",
    "            std::printf(\"%c%c\", ASCII_BACKSLASH, ASCII_DQUOTE);",
    "        } else {",
    "            std::printf(\"%c\", s[i]);",
    "        }",
    "    }",
    "}",
    "",
    "static void emit_data() {",
    "    for (int i = 0; i < LINE_COUNT; ++i) {",
    "        if (L[i] == nullptr) {",
    "            std::printf(\"    nullptr,\\n\");",
    "        } else {",
    "            std::printf(\"    %c\", ASCII_DQUOTE);",
    "            print_escaped(L[i]);",
    "            std::printf(\"%c,\\n\", ASCII_DQUOTE);",
    "        }",
    "    }",
    "}",
    "",
    "static void emit_source() {",
    "    for (int i = 0; i < LINE_COUNT; ++i) {",
    "        if (L[i] == nullptr) {",
    "            emit_data();",
    "        } else {",
    "            std::printf(\"%s\\n\", L[i]);",
    "        }",
    "    }",
    "}",
    "",
    "static void run_tests_and_demonstration() {",
    "    int null_count = 0;",
    "    bool has_printf = false;",
    "    for (int i = 0; i < LINE_COUNT; ++i) {",
    "        if (L[i] == nullptr) {",
    "            ++null_count;",
    "        } else if (std::strstr(L[i], \"std::printf\") != nullptr) {",
    "            has_printf = true;",
    "        }",
    "    }",
    "    // test: exactly one nullptr marker for self-reference",
    "    assert(null_count == 1);",
    "    // test: source uses std::printf as required by task",
    "    assert(has_printf);",
    "    // test: first line is compilation comment",
    "    assert(std::strncmp(L[0], \"//g++\", 5) == 0);",
    "    // test: last line closes main function",
    "    assert(std::strcmp(L[LINE_COUNT - 1], \"}\") == 0);",
    "    // test: required header cstdio is present",
    "    assert(std::strcmp(L[1], \"#include <cstdio>\") == 0);",
    "    std::printf(\"All tests passed.\\n\");",
    "    emit_source();",
    "}",
    "",
    "int main() {",
    "    run_tests_and_demonstration();",
    "    return 0;",
    "}",
};

static constexpr int LINE_COUNT = static_cast<int>(sizeof(L) / sizeof(L[0]));

static void print_escaped(const char *s) {
    for (int i = 0; s[i] != 0; ++i) {
        if (s[i] == ASCII_BACKSLASH) {
            std::printf("%c%c", ASCII_BACKSLASH, ASCII_BACKSLASH);
        } else if (s[i] == ASCII_DQUOTE) {
            std::printf("%c%c", ASCII_BACKSLASH, ASCII_DQUOTE);
        } else {
            std::printf("%c", s[i]);
        }
    }
}

static void emit_data() {
    for (int i = 0; i < LINE_COUNT; ++i) {
        if (L[i] == nullptr) {
            std::printf("    nullptr,\n");
        } else {
            std::printf("    %c", ASCII_DQUOTE);
            print_escaped(L[i]);
            std::printf("%c,\n", ASCII_DQUOTE);
        }
    }
}

static void emit_source() {
    for (int i = 0; i < LINE_COUNT; ++i) {
        if (L[i] == nullptr) {
            emit_data();
        } else {
            std::printf("%s\n", L[i]);
        }
    }
}

static void run_tests_and_demonstration() {
    int null_count = 0;
    bool has_printf = false;
    for (int i = 0; i < LINE_COUNT; ++i) {
        if (L[i] == nullptr) {
            ++null_count;
        } else if (std::strstr(L[i], "std::printf") != nullptr) {
            has_printf = true;
        }
    }
    // test: exactly one nullptr marker for self-reference
    assert(null_count == 1);
    // test: source uses std::printf as required by task
    assert(has_printf);
    // test: first line is compilation comment
    assert(std::strncmp(L[0], "//g++", 5) == 0);
    // test: last line closes main function
    assert(std::strcmp(L[LINE_COUNT - 1], "}") == 0);
    // test: required header cstdio is present
    assert(std::strcmp(L[1], "#include <cstdio>") == 0);
    std::printf("All tests passed.\n");
    emit_source();
}

int main() {
    run_tests_and_demonstration();
    return 0;
}
