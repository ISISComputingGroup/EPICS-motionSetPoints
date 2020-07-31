#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "setPoint.hpp"

using ::testing::_;
using ::testing::StrEq;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::ReturnNull;
using ::testing::InSequence;

ACTION_TEMPLATE(SetArgNPointeeTo, HAS_1_TEMPLATE_PARAMS(unsigned, uIndex), AND_1_VALUE_PARAMS(data))
{
    auto buffer = std::get<uIndex>(args);
    strcpy(buffer, data.c_str());
    return buffer;
}

namespace {
    class MockFileIO : public FileIOInterface {
    public:
        virtual ~MockFileIO() { }

        MOCK_METHOD2(Open, FILE*(const char* filename, const char* mode));
        MOCK_METHOD3(Read, char*(char *str, int n, FILE *stream));
        MOCK_METHOD1(Close, int(FILE* file));
    };

    TEST(setPoint, GIVEN_number_of_coords_set_to_10_WHEN_get_number_of_coords_THEN_return_10){
        auto envFilename = "TEST";
        setNumCoords(envFilename, 10);
        ASSERT_EQ(getNumCoords(envFilename), 10);
    }

    TEST(setPoint, GIVEN_non_existant_file_WHEN_loadFile_called_THEN_file_not_read) {
        auto testFilename = "my_file";
        MockFileIO mockFile;

        {
            InSequence seq;
            EXPECT_CALL(mockFile, Open(StrEq(testFilename), StrEq("rt"))).WillOnce(ReturnNull());

            EXPECT_CALL(mockFile, Read(_, _, _)).Times(0);
        }

        loadFile(&mockFile, testFilename, "TEST");
    }

    void createMockFile(MockFileIO* mockFile, std::string testFileName, std::vector<std::string> linesInFile, bool expectedToGetToEnd=true) {
        FILE testFile;

        InSequence seq;
        EXPECT_CALL(*mockFile, Open(StrEq(testFileName), StrEq("rt"))).WillOnce(Return(&testFile));

        for (auto line : linesInFile) {
            EXPECT_CALL(*mockFile, Read(_, _, _)).WillOnce(SetArgNPointeeTo<0>(line));
        }

        if (expectedToGetToEnd) {
            EXPECT_CALL(*mockFile, Read(_, _, _)).WillOnce(ReturnNull());
        }

        EXPECT_CALL(*mockFile, Close(_)).Times(1).WillOnce(Return(0));
    }

    TEST(setPoint, GIVEN_empty_file_WHEN_loadFile_called_THEN_file_read_and_closed) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        MockFileIO mockFile;
        std::vector<std::string> fileLines = {};
        
        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename);
    }

    TEST(setPoint, GIVEN_file_with_comments_in_header_WHEN_loadFile_called_THEN_comments_not_read) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "#Position    2" };
        MockFileIO mockFile;

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 0);
    }

    TEST(setPoint, GIVEN_file_with_single_row_and_single_coord_WHEN_loadFile_called_THEN_single_row_and_coord_with_correct_data) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position    2" };
        MockFileIO mockFile;
        
        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 1);
        ASSERT_EQ(table.rows[0].x, 2);

        ASSERT_EQ(getNumCoords(envFilename), 1);
    }

    TEST(setPoint, GIVEN_file_with_single_row_and_two_coords_WHEN_loadFile_called_THEN_single_row_and_coord_with_correct_data) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position    2    9" };
        MockFileIO mockFile;
        
        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 1);
        ASSERT_EQ(table.rows[0].x, 2);
        ASSERT_EQ(table.rows[0].y, 9);

        ASSERT_EQ(getNumCoords(envFilename), 2);
    }

    TEST(setPoint, GIVEN_file_with_multiple_rows_and_single_coord_WHEN_loadFile_called_THEN_multiple_rows_and_coord_with_correct_data) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position    2", "Position_2    7" };
        MockFileIO mockFile;

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 2);
        ASSERT_EQ(table.rows[0].x, 2);
        ASSERT_EQ(table.rows[1].x, 7);

        ASSERT_EQ(getNumCoords(envFilename), 1);
    }

    TEST(setPoint, GIVEN_file_with_inconsistent_coord_WHEN_loadFile_called_THEN_table_cleared) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position    2", "Position_2    7    9" };
        MockFileIO mockFile;

        createMockFile(&mockFile, testFilename, fileLines, false);

        loadFile(&mockFile, testFilename, envFilename);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 0);
    }

    TEST(setPoint, GIVEN_file_with_multiple_positions_with_the_same_name_WHEN_loadFile_called_THEN_table_cleared) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position    2", "Position    7" };
        MockFileIO mockFile;

        createMockFile(&mockFile, testFilename, fileLines, false);

        loadFile(&mockFile, testFilename, envFilename);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 0);
    }

    TEST(setPoint, GIVEN_file_with_multiple_names_for_the_same_position_WHEN_loadFile_called_THEN_table_cleared) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position    2    8", "Position_1    2    8" };
        MockFileIO mockFile;

        createMockFile(&mockFile, testFilename, fileLines, false);

        loadFile(&mockFile, testFilename, envFilename);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 0);
    }

} // namespace
