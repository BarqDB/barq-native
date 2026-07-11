#include "../main.hpp"
#include "test_objects.hpp"

#include <sstream>
#include <string>

using namespace barq::native;

// db.hpp defines a templated `operator<<(std::ostream&, const T*&)` for
// debug-printing object links. It used to match *any* pointer, so with a
// `using namespace barq::native;` in scope a plain `const char*` lvalue became
// ambiguous with the standard library's `const char*` overload. It is now
// constrained to Barq schema-object pointers only.

// Compile-time: the constraint admits schema objects and nothing else. The
// `char` case is the important one — `managed<char>` is never defined, so the
// trait must stay SFINAE-safe instead of hard-erroring on an incomplete type.
static_assert(internal::is_barq_object<Person>::value,
              "Person is a BARQ_SCHEMA object");
static_assert(!internal::is_barq_object<char>::value,
              "char is not a Barq object (and managed<char> is incomplete)");
static_assert(!internal::is_barq_object<int64_t>::value,
              "int64_t is not a Barq object");
static_assert(!internal::is_barq_object<std::string>::value,
              "std::string is not a Barq object");

TEST_CASE("const char* streaming is not hijacked by the link operator", "[link_stream]") {
    // Before the fix this line was an ambiguous-overload compile error, so the
    // test simply compiling is most of the guarantee.
    const char* msg = "plain c-string";
    std::ostringstream os;
    os << msg;
    CHECK(os.str() == "plain c-string");

    // The exact shape from the report — a cast expression.
    std::ostringstream os2;
    os2 << (const char*)"cast form";
    CHECK(os2.str() == "cast form");
}

TEST_CASE("schema-object pointer streaming still prints a link", "[link_stream]") {
    // The constrained operator still applies to object pointers. (A null
    // pointer is enough — we only care that our overload, not the standard
    // one, is selected, and that it no longer recurses forever.)
    const Person* p = nullptr;
    std::ostringstream os;
    os << p;
    CHECK(os.str().rfind("link:", 0) == 0);
}
