#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
Cell::Cell() { Clear(); }

Cell::~Cell() {}

void Cell::Set(std::string text) {
  if (text.empty()) {
    Clear();
  } else if (text.front() == '=' && text.size() > 1) {
    impl_ = std::make_unique<FormulaImpl>(text.substr(1));
  } else {
    impl_ = std::make_unique<TextImpl>(text);
  }
}

void Cell::Clear() { impl_ = std::make_unique<EmptyImpl>(); }

Cell::Value Cell::GetValue() const { return impl_->GetValue(); }
std::string Cell::GetText() const { return impl_->GetText(); }

Cell::Value Cell::TextImpl::GetValue() const {
  std::string value;
  if (text_.front() == '\'') {
    return text_.substr(1);
  }
  return text_;
}

std::string Cell::FormulaImpl::GetText() const {
  return '=' + formula_->GetExpression();
}

Cell::Value Cell::FormulaImpl::GetValue() const {
  auto evaluate_result = formula_->Evaluate();
  if (std::holds_alternative<double>(evaluate_result)) {
    return {std::get<double>(evaluate_result)};
  }
  return {std::get<FormulaError>(evaluate_result)};
}
