file( GLOB_RECURSE crrc_SRCS *.cpp *.h )

set( crrc_SRCS ${crrc_SRCS} )

# Create the application
add_executable(crrcTest ${crrc_SRCS} )

# Link to Cutelyst
target_link_libraries(crrcTest
    Qt5::Core
    Qt5::Network
    Qt5::Test
)

