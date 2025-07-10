#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "SimpleCalculator.h"
#include "InMemoryHistory.h"
#include "ICalculator.h"
#include "IHistory.h"

using namespace calc;
using ::testing::Return;
using ::testing::Ref;

// Мок для IHistory
class MockHistory : public IHistory {
public:
    MOCK_METHOD(void, AddEntry, (const std::string& operation), (override));
    MOCK_METHOD(std::vector<std::string>, GetLastOperations, (size_t count), (const, override));
};

// Мок для ICalculator
class MockCalculator : public ICalculator {
public:
    MOCK_METHOD(int, Add, (int a, int b), (override));
    MOCK_METHOD(int, Subtract, (int a, int b), (override));
    MOCK_METHOD(int, Multiply, (int a, int b), (override));
    MOCK_METHOD(int, Divide, (int a, int b), (override));
    MOCK_METHOD(void, SetHistory, (IHistory& history), (override));
};

// Фикстура для реального калькулятора с моком истории
class SimpleCalculatorTest : public ::testing::Test {
protected:
    MockHistory history;
    SimpleCalculator calculator{ history };
};

// Сложение
TEST_F(SimpleCalculatorTest, Add_ReturnsSumAndLogs) {
    EXPECT_CALL(history, AddEntry("2 + 2 = 4")).Times(1);
    EXPECT_EQ(calculator.Add(2, 2), 4);
}

// Сложение отрицательных
TEST_F(SimpleCalculatorTest, Add_Negatives) {
    EXPECT_CALL(history, AddEntry("-2 + -3 = -5")).Times(1);
    EXPECT_EQ(calculator.Add(-2, -3), -5);
}

// Вычитание
TEST_F(SimpleCalculatorTest, Subtract_ReturnsDiffAndLogs) {
    EXPECT_CALL(history, AddEntry("5 - 3 = 2")).Times(1);
    EXPECT_EQ(calculator.Subtract(5, 3), 2);
}

// Вычитание с отрицательным результатом
TEST_F(SimpleCalculatorTest, Subtract_NegativeResult) {
    EXPECT_CALL(history, AddEntry("3 - 5 = -2")).Times(1);
    EXPECT_EQ(calculator.Subtract(3, 5), -2);
}

// Умножение на ноль
TEST_F(SimpleCalculatorTest, Multiply_ByZero) {
    EXPECT_CALL(history, AddEntry("7 * 0 = 0")).Times(1);
    EXPECT_EQ(calculator.Multiply(7, 0), 0);
}

// Умножение положительного и отрицательного
TEST_F(SimpleCalculatorTest, Multiply_PositiveNegative) {
    EXPECT_CALL(history, AddEntry("-4 * 5 = -20")).Times(1);
    EXPECT_EQ(calculator.Multiply(-4, 5), -20);
}

// Умножение двух отрицательных
TEST_F(SimpleCalculatorTest, Multiply_NegativeNegative) {
    EXPECT_CALL(history, AddEntry("-4 * -5 = 20")).Times(1);
    EXPECT_EQ(calculator.Multiply(-4, -5), 20);
}

// Умножение больших значений (граница)
TEST_F(SimpleCalculatorTest, Multiply_LargeValues) {
    int a = 1000000;
    int b = 3000;
    int64_t expected = static_cast<int64_t>(a) * b;
    EXPECT_CALL(history, AddEntry(::testing::_)).Times(1);
    EXPECT_EQ(calculator.Multiply(a, b), expected);
}

// Деление без остатка
TEST_F(SimpleCalculatorTest, Divide_ReturnsQuotient) {
    EXPECT_CALL(history, AddEntry("10 / 2 = 5")).Times(1);
    EXPECT_EQ(calculator.Divide(10, 2), 5);
}

// Деление с остатком
TEST_F(SimpleCalculatorTest, Divide_WithRemainder) {
    EXPECT_CALL(history, AddEntry("7 / 2 = 3")).Times(1);
    EXPECT_EQ(calculator.Divide(7, 2), 3);
}

// Деление отрицательных
TEST_F(SimpleCalculatorTest, Divide_NegativeNumerator) {
    EXPECT_CALL(history, AddEntry("-10 / 3 = -3")).Times(1);
    EXPECT_EQ(calculator.Divide(-10, 3), -3);
}

// Деление на ноль
TEST_F(SimpleCalculatorTest, Divide_ByZero_NoThrow) {
    calculator.Divide(5, 0);  // упадёт при SEH‑исключении
}

// SetHistory не переназначает ссылку (логирование в мок)
TEST_F(SimpleCalculatorTest, SetHistory_DoesNotRebindReference) {
    InMemoryHistory mem;
    EXPECT_CALL(history, AddEntry("1 + 1 = 2")).Times(1);
    calculator.SetHistory(mem);
    calculator.Add(1, 1);
}

// Моки на интерфейс калькулятора и история
// Роль мока калькулятора: проверяем вызовы
TEST(MockCalculatorTest, CalculatorMock_AddCalled) {
    MockCalculator mockCalc;
    EXPECT_CALL(mockCalc, Add(2, 3)).Times(1).WillOnce(Return(5));
    EXPECT_EQ(mockCalc.Add(2, 3), 5);
}

// Мок калькулятора SetHistory вызывается
TEST(MockCalculatorTest, CalculatorMock_SetHistoryCalled) {
    MockCalculator mockCalc;
    InMemoryHistory realHistory;
    EXPECT_CALL(mockCalc, SetHistory(Ref(realHistory))).Times(1);
    mockCalc.SetHistory(realHistory);
}

// Мок калькулятора не влияет на реальную историю
TEST(MockCalculatorIntegration, MockCalcDoesNotLogToRealHistory) {
    MockCalculator mockCalc;
    InMemoryHistory realHistory;
    EXPECT_CALL(mockCalc, Divide(10, 5)).Times(1).WillOnce(Return(2));
    mockCalc.SetHistory(realHistory);
    EXPECT_EQ(mockCalc.Divide(10, 5), 2);
    EXPECT_TRUE(realHistory.GetLastOperations(1).empty());
}

// Тесты для InMemoryHistory
// Добавление и получение
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

// Проверка отсутствия переполнения истории
TEST(InMemoryHistoryTest, History_NoOverflow_AllEntriesStored) {
    InMemoryHistory history;
    const size_t kCount = 100;
    for (size_t i = 0; i < kCount; ++i) history.AddEntry("op" + std::to_string(i));
    auto all = history.GetLastOperations(kCount);
    ASSERT_EQ(all.size(), kCount);
    for (size_t i = 0; i < kCount; ++i) EXPECT_EQ(all[i], "op" + std::to_string(i));
}
