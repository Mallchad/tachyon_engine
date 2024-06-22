
#include "file.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>

    TEST_CASE( "File Handling Library" )
    {
        REQUIRE( file_load_binary( "/mnt/tmp/repos/experiments_rendering/assets/empty_file.bin" )
                    .size() >= 0 );


    }

int main( int argc, char** argv )
{
    file_load_binary( "/mnt/tmp/repos/experiments_rendering/assets/empty_file.bin" );
    int result = Catch::Session().run( argc, argv );

    return result;
}

int test_main( int argc, char** argv )
{
    return main( argc, argv );
}
