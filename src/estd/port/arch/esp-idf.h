// NOTE: None of this works.  Want a way to determine IDF version and react to it because
// espressif makes a lot of breaking changes
#ifdef EXPERIMENTAL

#define IDF_VER_2_0_0_444 "v2.0.0-444-g0d718b2"

// compares two strings in compile time constant fashion
constexpr int c_strcmp( char const* lhs, char const* rhs )
{
    return (('\0' == lhs[0]) && ('\0' == rhs[0])) ? 0
        :  (lhs[0] != rhs[0]) ? (lhs[0] - rhs[0])
        : c_strcmp( lhs+1, rhs+1 );
}

#if (0 == c_strcmp( IDF_VER, IDF_VER_2_0_0_444 ))
# define ESTD_IDF_VER_MAJOR 2
# define ESTD_IDF_VER_MINOR 0
# define ESTD_IDF_VER_SUFFIX 444
#elif IDF_VER == "v2.0.0-644-g765754f"
# define ESTD_IDF_VER_MAJOR 2
# define ESTD_IDF_VER_MINOR 0
# define ESTD_IDF_VER_SUFFIX 644
#endif

#endif
