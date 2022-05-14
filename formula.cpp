#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe){
  output << fe.ToString();
  return output;
}

std::ostream& operator<<(std::ostream& output,
                                const CellInterface::Value& value) {
  std::visit([&](const auto& x) { output << x; }, value);
  return output;
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
 explicit Formula(std::string expression):ast_(ParseFormulaAST(expression)){ 
 }
 Value Evaluate(const SheetInterface& sheet) const override {
   try {
     return ast_.Execute(sheet);
   } catch (FormulaError& e) {
     return e;
   }   
 }
 std::string GetExpression() const override { 
   std::ostringstream buffer;
   ast_.PrintFormula(buffer);
   return buffer.str();
 }

 std::vector<Position> GetReferencedCells() const { 
   const auto& cells_list = ast_.GetCells();

   std::vector<Position> result{cells_list.cbegin(), cells_list.cend()};
   auto last = std::unique(result.begin(), result.end());
   result.erase(last, result.end());

   return result;
 }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
  try {
    return std::make_unique<Formula>(std::move(expression));
  } catch (...) {
    throw FormulaException("Parse error");
  }
}