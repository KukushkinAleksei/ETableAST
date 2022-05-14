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
  void FillReferencesCells();
  void ClearReferencedCells();
  void DeleteCache() const;
   
   class Impl {
   public:
    virtual ~Impl() {}
    virtual Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
    virtual void DeleteCache() = 0;
  };

  class EmptyImpl : public Impl {
   public:
    EmptyImpl() {}

    Value GetValue() const override { return 0.0; }

    std::string GetText() const override { return ""; }

    std::vector<Position> GetReferencedCells() const override { return {}; }

    void DeleteCache() override{};
  };

  class TextImpl : public Impl {
   public:
    TextImpl(std::string text) : text_(text) {}

    std::string GetText() const override { return text_; }

    Value GetValue() const override;

    std::vector<Position> GetReferencedCells() const override { return {}; }
    void DeleteCache() override {}

   private:
    std::string text_;
  };

  class FormulaImpl : public Impl {
   public:
    FormulaImpl(const SheetInterface& sheet, std::string text);

    std::string GetText() const override;

    CellInterface::Value CalculateFormula() const;

    Value GetValue() const override;

    std::vector<Position> GetReferencedCells() const override;

    void DeleteCache() override;

   private:
    const SheetInterface& sheet_;
    std::unique_ptr<FormulaInterface> formula_;
    mutable std::optional<CellInterface::Value> cache_;
  };

  std::unique_ptr<Impl> impl_;
  Sheet& sheet_;
  std::set<Cell*> dependent_cells_;  
  std::set<Cell*> referensed_cells_;
};
