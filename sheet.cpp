#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::ResizeDataUpToPos(const Position& pos) {
  if (pos.row >= static_cast<int>(data_.size())) {
    data_.resize(pos.row + 1);
    print_size_.rows = std::max(print_size_.rows, pos.row);
  }
  auto& row = data_.at(pos.row);
  if (pos.col >= static_cast<int>(row.size())) {
    row.resize(pos.col + 1);
    print_size_.cols = std::max(print_size_.cols, pos.col);
  }
}

Cell* Sheet::GetCellPtr(const Position& ref_pos) {
  return data_[ref_pos.row][ref_pos.col].get();
}

void Sheet::SetReferencedCells(std::vector<Position>& references,
                               const Position& pos) {
  for (const Position& ref_pos : references) {
    if (!ref_pos.IsValid()) {
      continue;
    }

    CellInterface* cell_interface = GetCell(ref_pos);
    if (cell_interface == nullptr) {
      // initialise referenced cell
      ResizeDataUpToPos(ref_pos);
      data_[ref_pos.row][ref_pos.col] = std::make_unique<Cell>(*this);
    }
    Cell* cell = GetCellPtr(ref_pos);
    cell->AddReferencedIn(pos);
  }
}

void Sheet::CheckCircularReferences(std::vector<Position>& references,
                                    Position& pos) {
  std::set<Position> stack_of_refs;
  stack_of_refs.insert(references.begin(), references.end());

  while (!stack_of_refs.empty()) {
    Position curent = *stack_of_refs.begin();
    stack_of_refs.erase(curent);

    Cell* ref_cell = GetCellPtr(curent);
    auto nested_refs = ref_cell->GetReferencedCells();
    stack_of_refs.insert(nested_refs.begin(), nested_refs.end());
    if (stack_of_refs.count(pos)) {
      throw CircularDependencyException("Circular Dependency in " +
                                        pos.ToString());
    }
  }
}

void Sheet::SetCell(Position pos, std::string text) {
  if (!pos.IsValid()) {
    throw InvalidPositionException("Invalid position");
  }

  auto tmp_cell = std::make_unique<Cell>(*this);
  tmp_cell->Set(text);

  ResizeDataUpToPos(pos);

  auto references = tmp_cell->GetReferencedCells();
  SetReferencedCells(references, pos);
  CheckCircularReferences(references, pos);

  data_[pos.row][pos.col] = std::move(tmp_cell);
}

const CellInterface* Sheet::GetCell(Position pos) const {
  if (!pos.IsValid()) {
    throw InvalidPositionException("Invalid position");
  }

  if (pos.row < static_cast<int>(data_.size())) {
    auto& row = data_.at(pos.row);
    if (pos.col < static_cast<int>(row.size())) {
      auto& cell = row.at(pos.col);
      if (cell) {
        return cell.get();
      }
    }
  }
  return nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
  return const_cast<CellInterface*>(
      const_cast<const Sheet&>(*this).GetCell(pos));
}

void Sheet::UpdatePrintableSize() {
  int max_row = -1;
  int max_col = -1;
  
  for (int row_idx = 0; row_idx < static_cast<int>(data_.size()); ++row_idx) {
    const auto& row = data_.at(row_idx);
    for (int col_idx = 0; col_idx < static_cast<int>(row.size()); ++col_idx) {
      if (col_idx < static_cast<int>(row.size())) {
        if (row.at(col_idx)) {
          max_row = std::max(max_row, row_idx);
          max_col = std::max(max_col, col_idx);
        }
      }
    }
  }
  print_size_ = {max_row, max_col};
}

void Sheet::ClearCell(Position pos) {
  if (!pos.IsValid()) {
    throw InvalidPositionException("Invalid position");
  }

  if (pos.row < static_cast<int>(data_.size())) {
    auto& row = data_.at(pos.row);
    if (pos.col < static_cast<int>(row.size())) {
      auto& cell = row.at(pos.col);
      if (cell) {
        cell.reset();
        UpdatePrintableSize();
      }
    }
  }
}

Size Sheet::GetPrintableSize() const {
  return {print_size_.rows + 1, print_size_.cols + 1};
}

void Sheet::PrintData(std::ostream& output, PrintType print_type) const {
  for (int row_idx = 0; row_idx < print_size_.rows + 1; ++row_idx) {
    for (int col_idx = 0; col_idx < print_size_.cols + 1; ++col_idx) {
      if (col_idx != 0) {
        output << '\t';
      }
      const auto& row = data_.at(row_idx);
      if (col_idx < static_cast<int>(row.size())) {
        const auto& cell = row.at(col_idx);
        if (cell) {
          switch (print_type) {
            case PrintType::VALUES:
              output << cell->GetValue();
              break;
            case PrintType::TEXT:
              output << cell->GetText();
              break;
          }
        }
      }
    }
    output << '\n';
  }
}

void Sheet::PrintValues(std::ostream& output) const {
  PrintData(output, PrintType::VALUES);
}
void Sheet::PrintTexts(std::ostream& output) const {
  PrintData(output, PrintType::TEXT);
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}