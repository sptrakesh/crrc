file(GLOB_RECURSE crrc_SRCS *.cpp *.h)

set(crrc_SRCS
    ${crrc_SRCS}
)

# Create the application
add_executable(crrcTest main.cpp controller/LoginTest.cpp controller/LoginTest.h)

# Link to Cutelyst
target_link_libraries(crrcTest
    Cutelyst::Core
    Cutelyst::StaticSimple
    Cutelyst::View::Grantlee
    Cutelyst::Utils::Sql
    Cutelyst::Session
    Cutelyst::Authentication
    Qt5::Core
    Qt5::Network
    Qt5::Sql
    Qt5::Test
)

