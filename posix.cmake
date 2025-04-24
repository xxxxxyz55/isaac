

add_compile_options(-O3 -fPIC -rdynamic)
add_compile_options(-Wall)
# add_compile_options(-g)
add_compile_options(-s)

if(YS_MAVX2)
add_definitions(-DYS_MAVX2=1)
add_compile_options(-mavx2)
endif()

if(YS_SUPPORT_SSL)
find_library(gm_crypto libcrypto.a extern_lib NO_DEFAULT_PATH)
find_library(gm_ssl libssl.a extern_lib NO_DEFAULT_PATH)
find_library(crypto libcrypto.so.1.1 /lib/x86_64-linux-gnu)
find_library(ssl libssl.so.1.1 /lib/x86_64-linux-gnu)
endif()
find_library(redis libhiredis.a extern_lib NO_DEFAULT_PATH)

file(GLOB utest_src CONFIGURE_DEPENDS
 "test/*.cpp"
)

add_executable(utest test/utest.cpp)
target_include_directories(utest PUBLIC isaac)
target_include_directories(utest PUBLIC 3rdparty)
target_link_libraries(utest pthread)
target_link_libraries(utest ${gm_ssl} ${gm_crypto} ${redis} dl m)

add_executable(islam test/islam.cpp)
target_include_directories(islam PUBLIC isaac)
target_include_directories(islam PUBLIC 3rdparty)
target_link_libraries(islam ${gm_ssl} ${gm_crypto} dl m pthread)

add_executable(tohex test/tohex.cpp)
target_include_directories(tohex PUBLIC isaac)
target_link_libraries(tohex pthread)

add_executable(md5 test/md5.cpp)
target_include_directories(md5 PUBLIC isaac)
target_link_libraries(md5 pthread)

add_executable(heart test/heart.cpp)
target_include_directories(heart PUBLIC isaac)
target_link_libraries(heart pthread ${gm_ssl} ${gm_crypto} dl m)

add_executable(ipop test/ipop.cpp)
target_include_directories(ipop PUBLIC isaac)
target_link_libraries(ipop pthread ${gm_ssl} ${gm_crypto} dl m)

add_executable(fmt_name test/fmt_name.cpp)
target_include_directories(fmt_name PUBLIC isaac)
target_link_libraries(fmt_name pthread)

# add_executable(proxy test/proxy.cpp)
# target_include_directories(proxy PUBLIC isaac)
# target_include_directories(proxy PUBLIC 3rdparty)
# target_link_libraries(proxy pthread)
# target_link_libraries(proxy ${gm_ssl} ${gm_crypto} dl m)

# add_executable(downloader test/downloader.cpp)
# target_include_directories(downloader PUBLIC isaac)
# target_include_directories(downloader PUBLIC 3rdparty)
# target_link_libraries(downloader pthread)
# # target_link_libraries(downloader ${gm_ssl} ${gm_crypto} dl m)
# target_link_libraries(downloader ${ssl} ${crypto} dl m)

# add_executable(cbhttpserver test/callback_httpserver.cpp)
# target_include_directories(cbhttpserver PUBLIC isaac)
# target_include_directories(cbhttpserver PUBLIC 3rdparty)
# target_link_libraries(cbhttpserver pthread)
# target_link_libraries(cbhttpserver ${gm_ssl} ${gm_crypto} dl m)

# add_executable(cbhttpclient test/callback_httpclient.cpp)
# target_include_directories(cbhttpclient PUBLIC isaac)
# target_include_directories(cbhttpclient PUBLIC 3rdparty)
# target_link_libraries(cbhttpclient pthread)
# target_link_libraries(cbhttpclient ${gm_ssl} ${gm_crypto} dl m)

add_subdirectory(sqlutil)