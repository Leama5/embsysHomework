#include <gtest/gtest.h>
#include "../TimeParser.h"

//VKO5 perustehtävä tehty ja lisäksi lisätty ylimääräisiä testikeissejä 1p verran.
//Nämä lisäjutut tehty: merkkijono aina 6 merkkiä, palautusarvo ei 0s, merkkijono ei null. 

// Test suite: TimeParserTest
TEST(TimeParserTest, TestCorrectTime) {
    char time_test[] = "000005";
    EXPECT_EQ(time_parse(time_test), 5 );    
}
TEST(TimeParserTest, TestMinimumSecond) {
    char time_test[] = "000000";
    EXPECT_EQ(time_parse(time_test), TIME_ZERO_ERROR );    
}
TEST(TimeParserTest, TestMaxSecond) {

    char time_test[] = "000059";
    EXPECT_EQ(time_parse(time_test), 59 );  
}
TEST(TimeParserTest, TestTooLargeSeconds) {

    char time_test[] = "000060";
    EXPECT_EQ(time_parse(time_test), TIME_VALUE_ERROR );  
}

TEST(TimeParserTest, TestMinimumMinutes) {
    char time_test[] = "000011";
    EXPECT_EQ(time_parse(time_test), 11 );    
}
TEST(TimeParserTest, TestMaxMinutes) {

    char time_test[] = "005900";
    EXPECT_EQ(time_parse(time_test), 3540 );  
}
TEST(TimeParserTest, TestTooLargeMinutes) {

    char time_test[] = "006000";
    EXPECT_EQ(time_parse(time_test), TIME_VALUE_ERROR );  
}
TEST(TimeParserTest, TestMinimumHours) {
    char time_test[] = "000011";
    EXPECT_EQ(time_parse(time_test), 11 );    
}
TEST(TimeParserTest, TestMaxHours) {

    char time_test[] = "230011";
    EXPECT_EQ(time_parse(time_test), 11 );  
}
TEST(TimeParserTest, TestTooLargeHours) {

    char time_test[] = "240000";
    EXPECT_EQ(time_parse(time_test), TIME_VALUE_ERROR );  
}
TEST(TimeParserTest, TestNegativeSeconds) {

    char time_test[] = "00000-6";
    EXPECT_EQ(time_parse(time_test), TIME_LEN_ERROR );  
}

TEST(TimeParserTest, TestTooShortString) {

    char time_test[] = "0077";
    EXPECT_EQ(time_parse(time_test),TIME_LEN_ERROR);

}
TEST(TimeParserTest, TestTooLongString) {

    char time_test[] = "00007700";
    EXPECT_EQ(time_parse(time_test),TIME_LEN_ERROR);

}
TEST(TimeParserTest, TestNullPointer){
    EXPECT_EQ(time_parse(NULL),TIME_ARRAY_ERROR);
}


// https://google.github.io/googletest/reference/testing.html
// https://google.github.io/googletest/reference/assertions.html
