# from here:
#
# https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md

function(get_project_warnings compiler_warnings)
    option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)

    set(MSVC_WARNINGS
        /W4 # Baseline reasonable warnings
        /w14242 # 'identifier': conversion from 'type1' to 'type1', possible loss of data
        /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible
        # loss of data
        /w14263 # 'function': member function does not override any base class virtual
        # member function
        /w14265 # 'classname': class has virtual functions, but destructor is not virtual
        # instances of this class may not be destructed correctly
        /w14287 # 'operator': unsigned/negative constant mismatch
        /we4289 # nonstandard extension used: 'variable': loop control variable declared in the
        # for-loop is used outside the for-loop scope
        /w14296 # 'operator': expression is always 'boolean_value'
        /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
        /w14545 # expression before comma evaluates to a function which is missing an argument list
        /w14546 # function call before comma missing argument list
        /w14547 # 'operator': operator before comma has no effect; expected operator
        # with side-effect
        /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
        /w14555 # expression has no effect; expected expression with side- effect
        /w14619 # pragma warning: there is no warning number 'number'
        /w14640 # Enable warning on thread un-safe static member initialization
        /w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected
        # runtime behavior.
        /w14905 # wide string literal cast to 'LPSTR'
        /w14906 # string literal cast to 'LPWSTR'
        /w14928 # illegal copy-initialization; more than one user-defined conversion has been
        # implicitly applied
        /permissive- # standards conformance mode for MSVC compiler.
        # Disabled:
        /wd4715 # turn off: not all control paths return a value. Not good with switches
        /wd4646 # turn off: function declared with 'noreturn' has non-void return type
        )

    set(CLANG_WARNINGS
        -Weverything

        # NOT USEFUL
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-pre-c++17-compat
        -Wno-global-constructors
        -Wno-exit-time-destructors
        -Wno-switch-enum
        -Wno-padded
        -Wno-weak-vtables
        -Wno-disabled-macro-expansion

        # COULD BE USEFUL but cause existing code (e.g., math20) to have warnings
        -Wno-reserved-identifier
        -Wno-float-equal
        -Wno-date-time
        )

    if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        set(CLANG_WARNINGS
            ${CLANG_WARNINGS}
            -Wno-poison-system-directories # some issue with Freetype2 include directory with MacOS
            -Wno-missing-prototypes # doesn't pick up nested namespace inside anonymous namespace
            )
    endif ()

    set(GCC_WARNINGS
        -Wall
        -Wextra # reasonable and standard
        -Wpedantic # warn if non-standard C++ is used

        # OTHER USEFUL WARNINGS
        -Wcast-align # warn for potential performance problem casts
        -Wcast-qual
        -Wconversion # warn on type conversions that may lose data
        -Wdisabled-optimization
        -Wdouble-promotion # warn if float is implicit promoted to double
        -Wduplicated-branches # warn if if / else branches have duplicated code
        -Wduplicated-cond # warn if if / else chain has duplicated conditions
        -Wformat=2 # warn on security issues around functions that format output (ie printf)
        -Wimplicit-fallthrough # warn on statements that fallthrough without an explicit annotation
        -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
        -Wmissing-include-dirs # warn if a user-supplied include dir does not exist
        -Wmain # warn if the type of 'main' is suspicious
        -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
        -Wmissing-declarations
        -Wmissing-include-dirs
        -Wmissing-noreturn
        -Wnarrowing
        -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual
        # destructor. This helps catch hard to track down memory errors
        -Wnull-dereference # warn if paths that dereference a null pointer are detected
        -Wold-style-cast # warn for c-style casts
        -Woverloaded-virtual # warn if you overload (not override) a virtual function
        -Wpacked
        -Wpointer-arith # warn about sizeof for function types or void
        -Wredundant-decls
        -Wshadow # warn the user if a variable declaration shadows one from a parent context
        -Wsign-conversion # warn on sign conversions
        -Wsuggest-override
        -Wuninitialized
        -Wunsafe-loop-optimizations
        -Wunused # warn on anything being unused
        -Wuseless-cast # warn if you perform a cast to the same type
        -Wvla # warn about using variable-length arrays
        -Wzero-as-null-pointer-constant
        -fvisibility=hidden
        -fvisibility-inlines-hidden

        # NOT USEFUL
        -Wno-date-time
        -Wno-return-type
        -Wno-padded
        -Wno-switch-enum
        -Wno-float-equal
        )
    # Inline warning requires nasty destructors to be added as fixes.
    # if (CMAKE_CXX_COMPILER_ID MATCHES ".*GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12.1)
    #     set(GCC_WARNINGS "${GCC_WARNINGS}"
    #         -Winline
    #         --param max-inline-insns-single=3000  # give some more room before -Winline kicks in
    #         )
    # endif ()

    if (WARNINGS_AS_ERRORS)
        set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
        set(GCC_WARNINGS ${GCC_WARNINGS} -Werror)
        set(MSVC_WARNINGS ${MSVC_WARNINGS} /WX)
    endif ()

    if (MSVC)
        set(PROJECT_WARNINGS ${MSVC_WARNINGS})
    elseif (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        set(PROJECT_WARNINGS ${CLANG_WARNINGS})
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(PROJECT_WARNINGS ${GCC_WARNINGS})
    else ()
        message(AUTHOR_WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
    endif ()

    SET(${compiler_warnings} ${PROJECT_WARNINGS} PARENT_SCOPE)
endfunction()


function(set_project_warnings project_name)
    option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)

    SET(PROJECT_WARNINGS "")
    get_project_warnings(PROJECT_WARNINGS)

    target_compile_options(${project_name} PRIVATE ${PROJECT_WARNINGS})
endfunction()
