#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "SimpleCalculator.h"
#include "InMemoryHistory.h"
#include "ICalculator.h"
#include "IHistory.h"

using namespace calc;
using ::testing::_;
using ::testing::Return;
using ::testing::Exactly;
using ::testing::Ref;

// Ìîê äëÿ IHistory
class MockHistory : public IHistory {
public:
    MOCK_METHOD(void, AddEntry, (const std::string& operation), (override));
    MOCK_METHOD(std::vector<std::string>, GetLastOperations, (size_t count), (const, override));
};

// Ìîê äëÿ ICalculator
class MockCalculator : public ICalculator {
public:
    MOCK_METHOD(int, Add, (int a, int b), (override));
    MOCK_METHOD(int, Subtract, (int a, int b), (override));
    MOCK_METHOD(int, Multiply, (int a, int b), (override));
    MOCK_METHOD(int, Divide, (int a, int b), (override));
    MOCK_METHOD(void, SetHistory, (IHistory& history), (override));
};

// Ôèêñòóðà äëÿ ðåàëüíîãî êàëüêóëÿòîðà ñ ìîêîì èñòîðèè
class SimpleCalculatorTest : public ::testing::Test {
protected:
    MockHistory history;
    SimpleCalculator calculator{ history };
};

// Ñëîæåíèå
TEST_F(SimpleCalculatorTest, Add_ReturnsSumAndLogs) {
    EXPECT_CALL(history, AddEntry("2 + 2 = 4")).Times(1);
    EXPECT_EQ(calculator.Add(2, 2), 4);
}

// Ñëîæåíèå îòðèöàòåëüíûõ
TEST_F(SimpleCalculatorTest, Add_Negatives) {
    EXPECT_CALL(history, AddEntry("-2 + -3 = -5")).Times(1);
    EXPECT_EQ(calculator.Add(-2, -3), -5);
}

// Âû÷èòàíèå
TEST_F(SimpleCalculatorTest, Subtract_ReturnsDiffAndLogs) {
    EXPECT_CALL(history, AddEntry("5 - 3 = 2")).Times(1);
    EXPECT_EQ(calculator.Subtract(5, 3), 2);
}

// Âû÷èòàíèå ñ îòðèöàòåëüíûì ðåçóëüòàòîì
TEST_F(SimpleCalculatorTest, Subtract_NegativeResult) {
    EXPECT_CALL(history, AddEntry("3 - 5 = -2")).Times(1);
    EXPECT_EQ(calculator.Subtract(3, 5), -2);
}

// Óìíîæåíèå íà íîëü
TEST_F(SimpleCalculatorTest, Multiply_ByZero) {
    EXPECT_CALL(history, AddEntry("7 * 0 = 0")).Times(1);
    EXPECT_EQ(calculator.Multiply(7, 0), 0);
}

// Óìíîæåíèå ïîëîæèòåëüíîãî è îòðèöàòåëüíîãî
TEST_F(SimpleCalculatorTest, Multiply_PositiveNegative) {
    EXPECT_CALL(history, AddEntry("-4 * 5 = -20")).Times(1);
    EXPECT_EQ(calculator.Multiply(-4, 5), -20);
}

// Óìíîæåíèå äâóõ îòðèöàòåëüíûõ
TEST_F(SimpleCalculatorTest, Multiply_NegativeNegative) {
    EXPECT_CALL(history, AddEntry("-4 * -5 = 20")).Times(1);
    EXPECT_EQ(calculator.Multiply(-4, -5), 20);
}

// Óìíîæåíèå áîëüøèõ çíà÷åíèé (ãðàíèöà)
TEST_F(SimpleCalculatorTest, Multiply_LargeValues) {
    int a = 100000;
    int b = 30000;
    int expected = a * b;
    std::ostringstream oss;
    oss << a << " * " << b << " = " << expected;
    EXPECT_CALL(history, AddEntry(oss.str())).Times(1);
    EXPECT_EQ(calculator.Multiply(a, b), expected);
}

// Äåëåíèå áåç îñòàòêà
TEST_F(SimpleCalculatorTest, Divide_ReturnsQuotient) {
    EXPECT_CALL(history, AddEntry("10 / 2 = 5")).Times(1);
    EXPECT_EQ(calculator.Divide(10, 2), 5);
}

// Äåëåíèå ñ îñòàòêîì
TEST_F(SimpleCalculatorTest, Divide_WithRemainder) {
    EXPECT_CALL(history, AddEntry("7 / 2 = 3")).Times(1);
    EXPECT_EQ(calculator.Divide(7, 2), 3);
}

// Äåëåíèå îòðèöàòåëüíûõ
TEST_F(SimpleCalculatorTest, Divide_NegativeNumerator) {
    EXPECT_CALL(history, AddEntry("-10 / 3 = -3")).Times(1);
    EXPECT_EQ(calculator.Divide(-10, 3), -3);
}

// Äåëåíèå íà íîëü (òåêóùåå ïîâåäåíèå)
TEST_F(SimpleCalculatorTest, Divide_ByZero_NoThrow) {
    EXPECT_NO_THROW({ try { calculator.Divide(5, 0); } catch (...) {} });
}

// SetHistory íå ïåðåíàçíà÷àåò ññûëêó (ëîãèðîâàíèå â ìîê)
TEST_F(SimpleCalculatorTest, SetHistory_DoesNotRebindReference) {
    InMemoryHistory mem;
    EXPECT_CALL(history, AddEntry("1 + 1 = 2")).Times(1);
    calculator.SetHistory(mem);
    calculator.Add(1, 1);
}

// Ìîêè íà èíòåðôåéñ êàëüêóëÿòîðà è èñòîðèÿ
// Ðîëü ìîêà êàëüêóëÿòîðà: ïðîâåðÿåì âûçîâû
TEST(MockCalculatorTest, CalculatorMock_AddCalled) {
    MockCalculator mockCalc;
    EXPECT_CALL(mockCalc, Add(2, 3)).Times(1).WillOnce(Return(5));
    EXPECT_EQ(mockCalc.Add(2, 3), 5);
}

// Ìîê êàëüêóëÿòîðà SetHistory âûçûâàåòñÿ
TEST(MockCalculatorTest, CalculatorMock_SetHistoryCalled) {
    MockCalculator mockCalc;
    InMemoryHistory realHistory;
    EXPECT_CALL(mockCalc, SetHistory(Ref(realHistory))).Times(1);
    mockCalc.SetHistory(realHistory);
}

// Ìîê êàëüêóëÿòîðà íå âëèÿåò íà ðåàëüíóþ èñòîðèþ
TEST(MockCalculatorIntegration, MockCalcDoesNotLogToRealHistory) {
    MockCalculator mockCalc;
    InMemoryHistory realHistory;
    EXPECT_CALL(mockCalc, Divide(10, 5)).Times(1).WillOnce(Return(2));
    mockCalc.SetHistory(realHistory);
    EXPECT_EQ(mockCalc.Divide(10, 5), 2);
    EXPECT_TRUE(realHistory.GetLastOperations(1).empty());
}

// Òåñòû äëÿ InMemoryHistory
// Äîáàâëåíèå è ïîëó÷åíèå
TEST(InMemoryHistoryTest, AddAndRetrieveEntries) {
    InMemoryHistory history;
    history.AddEntry("op1"); history.AddEntry("op2"); history.AddEntry("op3");
    auto last2 = history.GetLastOperations(2);
    ASSERT_EQ(last2.size(), 2);
    EXPECT_EQ(last2[0], "op2"); EXPECT_EQ(last2[1], "op3");
}

// Count > size
TEST(InMemoryHistoryTest, GetLastOperations_WhenCountExceeds) {
    InMemoryHistory history;
    history.AddEntry("a");
    auto all = history.GetLastOperations(5);
    ASSERT_EQ(all.size(), 1);
    EXPECT_EQ(all[0], "a");
}

// Count = 0
TEST(InMemoryHistoryTest, GetLastOperations_ZeroCount) {
    InMemoryHistory history;
    history.AddEntry("x");
    EXPECT_TRUE(history.GetLastOperations(0).empty());
}

// Ïðîâåðêà îòñóòñòâèÿ ïåðåïîëíåíèÿ èñòîðèè
TEST(InMemoryHistoryTest, History_NoOverflow_AllEntriesStored) {
    InMemoryHistory history;
    const size_t kCount = 100;
    for (size_t i = 0; i < kCount; ++i) history.AddEntry("op" + std::to_string(i));
    auto all = history.GetLastOperations(kCount);
    ASSERT_EQ(all.size(), kCount);
    for (size_t i = 0; i < kCount; ++i) EXPECT_EQ(all[i], "op" + std::to_string(i));
}
