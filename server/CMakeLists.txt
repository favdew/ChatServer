cmake_minimum_required( VERSION 3.1 )

include_directories( . 
                    $ENV{BOOST_PATH}
                    )

set ( BOOST_LIB_PATH $ENV{BOOST_PATH}/stage/lib/ )
set ( boost_libs ${BOOST_LIB_PATH}libboost_system.a )

set ( mysql_libs mysqlcppconn )

set ( cryptopp_libs cryptopp )

find_package(Threads)

add_library( chat_application STATIC application/ThreadQueue.h
                application/ThreadQueue.cpp
                application/ThreadedSet.hpp
                application/ServerTerminal.h
                application/ServerTerminal.cpp
                application/crypto_proc.h
                application/crypto_proc.cpp
                application/Logger.h
                application/Logger.cpp
)

target_link_libraries( chat_application ${cryptopp_libs} )

set_target_properties( chat_application PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ./application)

add_library( chat_network STATIC network/chat_server.h network/chat_server.cpp
                network/chat_session.h network/chat_session.cpp
                network/chat_participant.h network/chat_room.h network/chat_room.cpp
                network/chat_server_interface.h
)

target_link_libraries( chat_network chat_application )

set_target_properties( chat_network PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ./network )



add_library( chat_db STATIC db/connector.h db/connector.cpp db/account_proc.h
                db/account_proc.cpp )

set_target_properties( chat_db PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ./db )

add_compile_options( -pthread )

add_executable( winchatserver main.cpp)

target_link_libraries(winchatserver chat_application chat_network
                        chat_db ${mysql_libs} ${boost_libs} ${cryptopp_libs}
                        Threads::Threads
)