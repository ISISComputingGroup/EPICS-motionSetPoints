#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "setPoint.hpp"

using ::testing::_;
using ::testing::StrEq;
using ::testing::Return;

namespace {
    class MockFileIO : public FileIOInterface {
    public:
        virtual ~MockFileIO() { }

        MOCK_METHOD2(Open, FILE*(const char* filename, const char* mode));
        MOCK_METHOD3(Read, char*(char *str, int n, FILE *stream));
        MOCK_METHOD1(Close, int(FILE* file));
    };

    TEST(setPoint, GIVEN_number_of_coords_set_to_10_WHEN_get_number_of_coords_THEN_return_10){
        std::string testFilename = "my_file";
        setNumCoords(testFilename.c_str(), 10);
        ASSERT_EQ(getNumCoords(testFilename.c_str()), 10);
    }

    TEST(setPoint, GIVEN_non_existant_file_WHEN_loadFile_called_THEN_file_not_read) {
        std::string testFilename = "my_file";
        MockFileIO mockFile;
        FILE testFile;

        EXPECT_CALL(mockFile, Open(StrEq(testFilename), StrEq("rt"))).WillOnce(Return(nullptr));

        EXPECT_CALL(mockFile, Read(_, _, _)).Times(0);

        loadFile(&mockFile, testFilename.c_str(), "TEST");
    }

} // namespace
