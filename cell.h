#pragma once

#include "common.h"
#include "formula.h"
#include <optional>
#include <set>

class Sheet;

class Cell : public CellInterface {
 public:
  explicit Cell(Sheet& sheet);
  ~Cell();

  void CheckForCycles(std::vector<Position>, Cell* root);

  void Set(std::string text);
  void Clear();

  Value GetValue() const override;
  std::string GetText() const override;
  std::vector<Position> GetReferencedCells() const override;
  bool IsReferenced() const;

 private:
  void FillReferencedCells();
  void ClearReferencedCells();
  void DeleteCache() const;

  class Impl;
  class EmptyImpl;
  class TextImpl;
  class FormulaImpl;
  
  std::unique_ptr<Impl> impl_;
  Sheet& sheet_;
  std::set<Cell*> dependent_cells_;  
  std::set<Cell*> referensed_cells_;
};
