add_executable(BTDownloader main.cpp)
target_link_libraries(BTDownloader SRCLIB BENCODING ${CURL_LIBRARIES})# ${requiredlibs})