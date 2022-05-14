#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
Cell::Cell(SheetInterface& sheet) : sheet_(sheet) { Clear(); }

Cell::~Cell() {}

void Cell::Set(std::string text) {
  if (text.empty()) {
    Clear();
  } else if (text.front() == FORMULA_SIGN && text.size() > 1) {
    impl_ = std::make_unique<FormulaImpl>(sheet_, text.substr(1));
  } else {
    impl_ = std::make_unique<TextImpl>(text);
  }
}

void Cell::Clear() { impl_ = std::make_unique<EmptyImpl>(); }

Cell::Value Cell::GetValue() const { return impl_->GetValue(); }
std::string Cell::GetText() const { return impl_->GetText(); }

std::vector<Position> Cell::GetReferencedCells() const {
  return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const { return referenced_in_.empty(); }

void Cell::AddReferencedIn(Position pos) { referenced_in_.insert(pos); }

Cell::Value Cell::TextImpl::GetValue() const {
  std::string value;
  if (text_.front() == ESCAPE_SIGN) {
    return text_.substr(1);
  }
  return text_;
}

Cell::FormulaImpl::FormulaImpl(const SheetInterface& sheet, std::string text)
    : sheet_(sheet) {
  try {
    formula_ = ParseFormula(text);
  } catch (...) {
    throw FormulaException("Failed to parse formula");
  }  
}

std::string Cell::FormulaImpl::GetText() const {
  return FORMULA_SIGN + formula_->GetExpression();
}

CellInterface::Value Cell::FormulaImpl::CalculateFormula() const {
  FormulaInterface::Value evaluate_result = formula_->Evaluate(sheet_);
  CellInterface::Value result;
  if (std::holds_alternative<double>(evaluate_result)) {
    result = std::get<double>(evaluate_result);
  } else if (std::holds_alternative<FormulaError>(evaluate_result)) {
    result = std::get<FormulaError>(evaluate_result);
  }
  return result;
}

Cell::Value Cell::FormulaImpl::GetValue() const {
  if (cache_ == std::nullopt) {
    cache_ = CalculateFormula();
  }
  return cache_.value();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
  return formula_->GetReferencedCells();
}

void Cell::FormulaImpl::DeleteCache() { cache_.reset(); }
