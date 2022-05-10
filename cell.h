#pragma once

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
 public:
  Cell();
  ~Cell();

  void Set(std::string text);
  void Clear();

  Value GetValue() const override;
  std::string GetText() const override;

 private:
  class Impl {
   public:
    virtual Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
  };

  class EmptyImpl : public Impl {
   public:
    EmptyImpl() {}
    Value GetValue() const override { return 0.0; }
    std::string GetText() const override { return ""; }
  };

  class TextImpl : public Impl {
   public:
    TextImpl(std::string text) : text_(text) {}
    std::string GetText() const override { return text_; }
    Value GetValue() const override;

   private:
    std::string text_;
  };

  class FormulaImpl : public Impl {
   public:
    FormulaImpl(std::string text) {
      formula_ = ParseFormula(text);
    }

    std::string GetText() const override;

    Value GetValue() const override;

   private:
    std::unique_ptr<FormulaInterface> formula_;
  };

  std::unique_ptr<Impl> impl_;
};