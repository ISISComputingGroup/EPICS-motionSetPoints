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

        loadFile(&mockFile, testFilename, "TEST", 1);
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

        loadFile(&mockFile, testFilename, envFilename, 0);
    }

    TEST(setPoint, GIVEN_file_with_comments_in_header_WHEN_loadFile_called_THEN_comments_not_read) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "#Position    2" };
        MockFileIO mockFile;

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename, 1);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 0);
    }

    TEST(setPoint, GIVEN_file_with_single_row_and_single_coord_WHEN_loadFile_called_THEN_single_row_and_coord_with_correct_data) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position    2" };
        MockFileIO mockFile;
        
        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename, 1);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 1);
        ASSERT_THAT(table.rows[0].coordinates, ElementsAre(2));
        ASSERT_STREQ(table.rows[0].name, "Position");
    }

    TEST(setPoint, GIVEN_file_with_single_row_and_three_coords_WHEN_loadFile_called_THEN_single_row_and_coord_with_correct_data) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position    2    9   8" };
        MockFileIO mockFile;
        
        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename, 3);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 1);
        ASSERT_THAT(table.rows[0].coordinates, ElementsAre(2, 9, 8));
        ASSERT_STREQ(table.rows[0].name, "Position");
    }

    TEST(setPoint, GIVEN_file_with_multiple_rows_and_single_coord_WHEN_loadFile_called_THEN_multiple_rows_and_coord_with_correct_data) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position    2", "Position_2    7" };
        MockFileIO mockFile;

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename, 1);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 2);
        ASSERT_THAT(table.rows[0].coordinates, ElementsAre(2));
        ASSERT_STREQ(table.rows[0].name, "Position");
        ASSERT_THAT(table.rows[1].coordinates, ElementsAre(7));
        ASSERT_STREQ(table.rows[1].name, "Position_2");
    }

    TEST(setPoint, GIVEN_file_with_unconvertable_decimals_WHEN_loadFile_called_THEN_table_cleared) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position    2,8" };
        MockFileIO mockFile;

        createMockFile(&mockFile, testFilename, fileLines, false);       

        loadFile(&mockFile, testFilename, envFilename, 1);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 0);
    }

    TEST(setPoint, GIVEN_file_with_inconsistent_coord_WHEN_loadFile_called_THEN_table_cleared) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position    2", "Position_2    7    9" };
        MockFileIO mockFile;

        createMockFile(&mockFile, testFilename, fileLines, false);

        loadFile(&mockFile, testFilename, envFilename, 1);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 0);
    }

    TEST(setPoint, GIVEN_file_with_multiple_positions_with_the_same_name_WHEN_loadFile_called_THEN_table_cleared) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position    2", "Position    7" };
        MockFileIO mockFile;

        createMockFile(&mockFile, testFilename, fileLines, false);

        loadFile(&mockFile, testFilename, envFilename, 1);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 0);
    }

    TEST(setPoint, GIVEN_file_with_multiple_names_for_the_same_position_WHEN_loadFile_called_THEN_table_cleared) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position    2    8", "Position_1    2    8" };
        MockFileIO mockFile;

        createMockFile(&mockFile, testFilename, fileLines, false);

        loadFile(&mockFile, testFilename, envFilename, 2);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 0);
    }

    TEST(setPoint, GIVEN_three_coordinate_position_which_differs_by_final_coordinate_WHEN_loaded_THEN_RBV_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position_42_78_98    42   78  98", "Position_42_78_15    42   78   15" };
        MockFileIO mockFile;
        double position;

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename, 3);

        auto table = getTable(envFilename);

        ASSERT_EQ(table.rows.size(), 2);
    }

    TEST(setPoint, GIVEN_single_coordinate_position_that_exactly_matches_existing_position_WHEN_posn2name_called_THEN_current_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position_42    42", "Position_39    39" };
        MockFileIO mockFile;
        double posDiff;
        char positionName[200];

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename, 1);

        std::vector<double> coords{ 42.0 };
        int positionFound = posn2name(coords, 0.1, envFilename, posDiff);

        ASSERT_EQ(positionFound, 0);
        ASSERT_EQ(posDiff, 0.0);

        positionFound = getPosnName(positionName, false, envFilename);
        ASSERT_STREQ(positionName, "Position_42");
        ASSERT_EQ(positionFound, 0);
    }

    TEST(setPoint, GIVEN_single_coordinate_position_that_matches_existing_position_within_tolerance_WHEN_posn2name_called_THEN_current_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position_42    42", "Position_39    39" };
        MockFileIO mockFile;
        double posDiff;
        char positionName[200];

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename, 1);

        std::vector<double> coords{ 42.1 };
        int positionFound = posn2name(coords, 0.2, envFilename, posDiff);

        ASSERT_EQ(positionFound, 0);
        ASSERT_NEAR(posDiff, 0.1, 0.0000001);

        positionFound = getPosnName(positionName, false, envFilename);
        ASSERT_EQ(positionFound, 0);
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

        loadFile(&mockFile, testFilename, envFilename, 1);

        std::vector<double> coords{ 40.0 };
        int positionFound = posn2name(coords, 0.1, envFilename, posDiff);

        ASSERT_EQ(positionFound, -1);

        positionFound = getPosnName(positionName, false, envFilename);
        ASSERT_EQ(positionFound, -1);
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

        loadFile(&mockFile, testFilename, envFilename, 2);

        std::vector<double> coords{ 42.0, 78.0 };
        int positionFound = posn2name(coords, 0.1, envFilename, posDiff);

        ASSERT_EQ(positionFound, 0);
        ASSERT_EQ(posDiff, 0.0);

        positionFound = getPosnName(positionName, false, envFilename);
        ASSERT_EQ(positionFound, 0);
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

        loadFile(&mockFile, testFilename, envFilename, 2);

        std::vector<double> coords{ 42.1, 77.9 };
        int positionFound = posn2name(coords, 0.2, envFilename, posDiff);

        ASSERT_EQ(positionFound, 0);
        ASSERT_NEAR(posDiff, pow(pow(0.1, 2) + pow(0.1, 2), 0.5), 0.0000001);

        positionFound = getPosnName(positionName, false, envFilename);
        ASSERT_EQ(positionFound, 0);
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

        loadFile(&mockFile, testFilename, envFilename, 2);

        std::vector<double> coords{ 40.0, 77 };
        int positionFound = posn2name(coords, 0.1, envFilename, posDiff);

        ASSERT_EQ(positionFound, -1);

        positionFound = getPosnName(positionName, false, envFilename);
        ASSERT_EQ(positionFound, -1);
        ASSERT_STREQ(positionName, "");
    }

    TEST(setPoint, GIVEN_three_coordinate_position_that_matches_existing_position_within_tolerance_WHEN_posn2name_called_THEN_current_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position_42_78_98    42   78  98", "Position_39_63_15    39   63   15" };
        MockFileIO mockFile;
        double posDiff;
        char positionName[200];
        std::vector<double> position {39.1, 63.1, 14.9};

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename, 3);

        int positionFound = posn2name(position, 0.2, envFilename, posDiff);

        ASSERT_EQ(positionFound, 0);
        ASSERT_NEAR(posDiff, pow(3 * pow(0.1, 2), 0.5), 0.0000001);

        positionFound = getPosnName(positionName, false, envFilename);
        ASSERT_EQ(positionFound, 0);
        ASSERT_STREQ(positionName, "Position_39_63_15");
    }

    TEST(setPoint, GIVEN_name_for_three_coordinate_position_WHEN_name2posn_called_THEN_RBV_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position_42_78_98    42   78  98", "Position_39_63_15    39   63   15" };
        MockFileIO mockFile;
        double position1, position2, position3;

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename, 3);

        int positionFound = name2posn("Position_42_78_98", envFilename);

        ASSERT_EQ(positionFound, 0);

        getPosn(0, true, envFilename, position1);
        ASSERT_DOUBLE_EQ(position1, 42.0);
        getPosn(1, true, envFilename, position2);
        ASSERT_DOUBLE_EQ(position2, 78.0);
        getPosn(2, true, envFilename, position3);
        ASSERT_DOUBLE_EQ(position3, 98.0);
    }

    TEST(setPoint, GIVEN_name_for_single_coord_position_WHEN_name2posn_called_THEN_RBV_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position_42    42", "Position_39    39" };
        MockFileIO mockFile;
        double position;

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename, 1);

        int positionFound = name2posn("Position_42", envFilename);

        ASSERT_EQ(positionFound, 0);

        getPosn(0, true, envFilename, position);
        ASSERT_DOUBLE_EQ(position, 42.0);
    }

    TEST(setPoint, GIVEN_lower_case_name_for_single_coord_position_WHEN_name2posn_called_THEN_RBV_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "PosItiOn_42    42", "Position_39    39" };
        MockFileIO mockFile;
        double position;

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename, 1);

        int positionFound = name2posn("position_42", envFilename);

        ASSERT_EQ(positionFound, 0);

        getPosn(0, true, envFilename, position);
        ASSERT_DOUBLE_EQ(position, 42.0);
    }

    TEST(setPoint, GIVEN_name_for_non_existant_single_coord_position_WHEN_name2posn_called_THEN_RBV_table_row_pointer_set_correctly) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "Position_42    42", "Position_39    39" };
        MockFileIO mockFile;
        double position;

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename, 1);

        int name2posnErrorCode = name2posn("Position_89", envFilename);

        ASSERT_EQ(name2posnErrorCode, -1);

        int getPosnErrorCode = getPosn(0, true, envFilename, position);
        ASSERT_EQ(getPosnErrorCode, -1);
    }

    TEST(setPoint, GIVEN_two_very_different_named_positions_WHEN_getPositions_called_THEN_returns_expected_string) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "short_name    42", "a_very_much_longer_name    39" };
        MockFileIO mockFile;
        double position;
        std::string positions;

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename, 1);

        getPositions(&positions, envFilename);

        ASSERT_EQ(positions, "short_name                              a_very_much_longer_name                 ");
    }

    TEST(setPoint, GIVEN_two_very_different_named_positions_in_different_order_WHEN_getPositions_called_THEN_returns_expected_string) {
        auto testFilename = "my_file";
        auto envFilename = "TEST";
        std::vector<std::string> fileLines = { "a_very_much_longer_name    42", "short_name    39" };
        MockFileIO mockFile;
        double position;
        std::string positions;

        createMockFile(&mockFile, testFilename, fileLines);

        loadFile(&mockFile, testFilename, envFilename, 1);

        getPositions(&positions, envFilename);

        ASSERT_EQ(positions, "a_very_much_longer_name                 short_name                              ");
    }

} // namespace
