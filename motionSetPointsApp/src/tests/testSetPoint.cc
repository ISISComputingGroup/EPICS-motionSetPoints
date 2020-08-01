#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "setPoint.hpp"

using ::testing::_;
using ::testing::StrEq;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::ReturnNull;
using ::testing::InSequence;
using ::testing::DoAll;
using ::testing::ElementsAre;

namespace {
    class MockFileIO : public FileIOInterface {
    public:
        virtual ~MockFileIO() { }

        MOCK_METHOD1(Open, void(const char* filename));
        MOCK_METHOD1(ReadLine, bool(std::string &str));
        MOCK_METHOD0(Close, void());
        MOCK_METHOD0(isOpen, bool());
    };

    TEST(setPoint, GIVEN_number_of_coords_set_to_10_WHEN_get_number_of_coords_THEN_return_10){
        auto envFilename = "TEST";
        setNumCoords(envFilename, 10);
        ASSERT_EQ(getNumCoords(envFilename), 10);
    }

    TEST(setPoint, GIVEN_file_line_with_one_coord_WHEN_row_created_THEN_correct_name_and_coords) {
        std::string fileLine = "Position    2";
        auto row = createRowFromFileLine(fileLine);

        ASSERT_STREQ(row.name, "Position");
        ASSERT_EQ(row.coordinates.size(), 1);
        ASSERT_EQ(row.coordinates[0], 2);
    }

    TEST(setPoint, GIVEN_file_line_with_two_coords_WHEN_row_created_THEN_correct_name_and_coords) {
        std::string fileLine = "Position    2   7";
        auto row = createRowFromFileLine(fileLine);

        ASSERT_STREQ(row.name, "Position");
        ASSERT_EQ(row.coordinates.size(), 2);
        ASSERT_THAT(row.coordinates, ElementsAre(2, 7));
    }

    TEST(setPoint, GIVEN_file_line_with_7_coords_WHEN_row_created_THEN_correct_name_and_coords) {
        std::string fileLine = "Position 1.8 2.9 3.7 4.5 5.9 6.8 7.1";
        auto row = createRowFromFileLine(fileLine);

        ASSERT_STREQ(row.name, "Position");
        ASSERT_EQ(row.coordinates.size(), 7);
        ASSERT_THAT(row.coordinates, ElementsAre(1.8, 2.9, 3.7, 4.5, 5.9, 6.8, 7.1));
    }

    TEST(setPoint, GIVEN_non_existant_file_WHEN_loadFile_called_THEN_file_not_read) {
        auto testFilename = "my_file";
        MockFileIO mockFile;

        {
            InSequence seq;
            EXPECT_CALL(mockFile, Open(StrEq(testFilename)));
            EXPECT_CALL(mockFile, isOpen()).WillOnce(Return(false));

            EXPECT_CALL(mockFile, ReadLine(_)).Times(0);
        }

        loadFile(&mockFile, testFilename, "TEST");
    }

    void createMockFile(MockFileIO* mockFile, std::string testFileName, std::vector<std::string> linesInFile, bool expectedToGetToEnd=true) {
        InSequence seq;
        EXPECT_CALL(*mockFile, Open(StrEq(testFileName)));
        EXPECT_CALL(*mockFile, isOpen()).WillOnce(Return(true));

        for (auto line : linesInFile) {
            EXPECT_CALL(*mockFile, ReadLine(_)).WillOnce(DoAll(SetArgReferee<0>(line), Return(true)));
        }

        if (expectedToGetToEnd) {
            EXPECT_CALL(*mockFile, ReadLine(_)).WillOnce(Return(false));
        }

        EXPECT_CALL(*mockFile, Close()).Times(1);
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
        ASSERT_EQ(table.rows[0].coordinates[0], 2);

        ASSERT_EQ(getNumCoords(envFilename), 1);
    }

    TEST(setPoint, GIVEN_file_with_single_row_and_three_coords_WHEN_loadFile_called_THEN_single_row_and_coord_with_correct_data) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position    2    9   8" };
        MockFileIO mockFile;
        
        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 1);
        ASSERT_THAT(table.rows[0].coordinates, ElementsAre(2, 9, 8));

        ASSERT_EQ(getNumCoords(envFilename), 3);
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
        ASSERT_EQ(table.rows[0].coordinates[0], 2);
        ASSERT_EQ(table.rows[1].coordinates[0], 7);

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

    TEST(setPoint, GIVEN_single_coordinate_position_that_exactly_matches_existing_position_WHEN_posn2name_called_THEN_current_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position_42    42", "Position_39    39" };
        MockFileIO mockFile;
        double posDiff;
        char positionName[200];

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename);

        int positionFound = posn2name(42.0, 0.1, envFilename, posDiff);

        ASSERT_EQ(positionFound, 0);
        ASSERT_EQ(posDiff, 0.0);

        getPosnName(positionName, 0, envFilename);
        ASSERT_STREQ(positionName, "Position_42");
    }

    TEST(setPoint, GIVEN_single_coordinate_position_that_matches_existing_position_within_tolerance_WHEN_posn2name_called_THEN_current_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position_42    42", "Position_39    39" };
        MockFileIO mockFile;
        double posDiff;
        char positionName[200];

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename);

        int positionFound = posn2name(42.1, 0.2, envFilename, posDiff);

        ASSERT_EQ(positionFound, 0);
        ASSERT_NEAR(posDiff, 0.1, 0.0000001);

        getPosnName(positionName, 0, envFilename);
        ASSERT_STREQ(positionName, "Position_42");
    }

    TEST(setPoint, GIVEN_single_coordinate_position_that_does_not_match_existing_position_within_tolerance_WHEN_posn2name_called_THEN_current_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position_42    42", "Position_39    39" };
        MockFileIO mockFile;
        double posDiff;
        char positionName[200];

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename);

        int positionFound = posn2name(40.0, 0.1, envFilename, posDiff);

        ASSERT_EQ(positionFound, -1);

        getPosnName(positionName, 0, envFilename);
        ASSERT_STREQ(positionName, "");
    }

    TEST(setPoint, GIVEN_two_coordinate_position_that_exactly_matches_existing_position_WHEN_posn2name_called_THEN_current_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position_42_78    42   78", "Position_39_63    39   63" };
        MockFileIO mockFile;
        double posDiff;
        char positionName[200];

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename);

        int positionFound = posn2name(42.0, 78.0, 0.1, envFilename, posDiff);

        ASSERT_EQ(positionFound, 0);
        ASSERT_EQ(posDiff, 0.0);

        getPosnName(positionName, 0, envFilename);
        ASSERT_STREQ(positionName, "Position_42_78");
    }

    TEST(setPoint, GIVEN_two_coordinate_position_that_matches_existing_position_within_tolerance_WHEN_posn2name_called_THEN_current_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position_42_78    42   78", "Position_39_63    39   63" };
        MockFileIO mockFile;
        double posDiff;
        char positionName[200];

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename);

        int positionFound = posn2name(42.1, 77.9, 0.2, envFilename, posDiff);

        ASSERT_EQ(positionFound, 0);
        ASSERT_NEAR(posDiff, pow(pow(0.1, 2) + pow(0.1, 2), 0.5), 0.0000001);

        getPosnName(positionName, 0, envFilename);
        ASSERT_STREQ(positionName, "Position_42_78");
    }

    TEST(setPoint, GIVEN_two_coordinate_position_that_does_not_match_existing_position_within_tolerance_WHEN_posn2name_called_THEN_current_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position_42_78    42   78", "Position_39_63    39   63" };
        MockFileIO mockFile;
        double posDiff;
        char positionName[200];

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename);

        int positionFound = posn2name(40.0, 77, 0.1, envFilename, posDiff);

        ASSERT_EQ(positionFound, -1);

        getPosnName(positionName, 0, envFilename);
        ASSERT_STREQ(positionName, "");
    }

    TEST(setPoint, GIVEN_name_for_single_coord_position_WHEN_name2posn_called_THEN_RBV_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position_42    42", "Position_39    39" };
        MockFileIO mockFile;
        double position;

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename);

        int positionFound = name2posn("Position_42", envFilename);

        ASSERT_EQ(positionFound, 0);

        getPosn(1, 1, envFilename, position);
        ASSERT_DOUBLE_EQ(position, 42.0);
    }

    TEST(setPoint, GIVEN_lower_case_name_for_single_coord_position_WHEN_name2posn_called_THEN_RBV_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "PosItiOn_42    42", "Position_39    39" };
        MockFileIO mockFile;
        double position;

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename);

        int positionFound = name2posn("position_42", envFilename);

        ASSERT_EQ(positionFound, 0);

        getPosn(1, 1, envFilename, position);
        ASSERT_DOUBLE_EQ(position, 42.0);
    }

    TEST(setPoint, GIVEN_name_for_non_existant_single_coord_position_WHEN_name2posn_called_THEN_RBV_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position_42    42", "Position_39    39" };
        MockFileIO mockFile;
        double position;

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename);

        int positionFound = name2posn("Position_89", envFilename);

        ASSERT_EQ(positionFound, -1);

        positionFound = getPosn(1, 1, envFilename, position);
        ASSERT_EQ(positionFound, -1);
    }

} // namespace