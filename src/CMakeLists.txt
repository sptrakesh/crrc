file(GLOB_RECURSE crrc_SRCS *.cpp *.h)

set(crrc_SRCS
    ${crrc_SRCS}
    ${TEMPLATES_SRC}
)

# Create the application
add_library(crrc SHARED ${crrc_SRCS})

# Link to Cutelyst
target_link_libraries(crrc
    Cutelyst::Core
    Cutelyst::StaticSimple
    Cutelyst::View::Grantlee
    Cutelyst::Utils::Sql
    Cutelyst::Session
    Cutelyst::Authentication
    Qt5::Core
    Qt5::Network
    Qt5::Sql
)

