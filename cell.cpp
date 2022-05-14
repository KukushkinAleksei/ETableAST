#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
Cell::Cell(Sheet& sheet) : sheet_(sheet) { Clear(); }

Cell::~Cell() {}

void Cell::CheckForCycles(std::vector<Position> references, Cell* root) {

  for (const auto& parent_pos : references) {
    if (auto parent = dynamic_cast<Cell*>(sheet_.GetCell(parent_pos))) {
      if (parent == root) {
        throw CircularDependencyException("Cycle found!");
      }
      parent->CheckForCycles(parent->GetReferencedCells(), root);
    }
  }
}

void Cell::Set(std::string text) {
  if (text.empty()) {
    Clear();
  } else if (text.front() == FORMULA_SIGN && text.size() > 1) {
    auto tmp_impl = std::make_unique<FormulaImpl>(sheet_, text.substr(1));
    CheckForCycles(tmp_impl->GetReferencedCells(), this);
    impl_ = std::move(tmp_impl);
  } else {
    impl_ = std::make_unique<TextImpl>(text);
  }
  ClearReferencedCells();
  FillReferencesCells();
  DeleteCache();
}

void Cell::FillReferencesCells() {
  for (const Position& ref_pos : GetReferencedCells()) {
    if (!ref_pos.IsValid()) {
      continue;
    }
    CellInterface* cell_interface = sheet_.GetCell(ref_pos);
    if (cell_interface == nullptr) {
      sheet_.SetCell(ref_pos, "");
    }
    Cell* cell = sheet_.GetCellPtr(ref_pos);
    cell->dependent_cells_.insert(this);
    referensed_cells_.insert(cell);
  }
}

void Cell::ClearReferencedCells() {
  for (auto ref_cell : referensed_cells_) {
    ref_cell->dependent_cells_.erase(this);
  }
  referensed_cells_.clear();
}

void Cell::Clear() {
  impl_ = std::make_unique<EmptyImpl>();
  ClearReferencedCells();
  DeleteCache();
}

Cell::Value Cell::GetValue() const { return impl_->GetValue(); }
std::string Cell::GetText() const { return impl_->GetText(); }

std::vector<Position> Cell::GetReferencedCells() const {
  return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const { return dependent_cells_.empty(); }


void Cell::DeleteCache() const {
  impl_->DeleteCache();
  for (Cell* cell : dependent_cells_) {
    cell->DeleteCache();
  }
}

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
