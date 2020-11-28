#pragma once

//
// This file is distributed under the MIT License. See LICENSE.md for details.
//

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/ConstantRange.h"

#include "revng/ADT/ZipMapIterator.h"
#include "revng/Support/Debug.h"

using APIntVector = llvm::SmallVector<llvm::APInt, 4>;

struct APIntVectorKeyContainer {
  static int compare(const llvm::APInt &LHS, const llvm::APInt &RHS) {
    if (LHS == RHS)
      return 0;
    if (LHS.ult(RHS))
      return -1;
    else
      return 1;
  }
};

// Options:
//
// * Empty
// * [10,)
// * [0,10)

class ConstantRangeSetIterator {
private:
  llvm::APInt Current;
  const llvm::APInt *NextBound;
  const llvm::APInt *const End;
  bool ToLast;
  bool Done;
  const llvm::APInt Max;

public:
  ConstantRangeSetIterator(const llvm::APInt *Start, const llvm::APInt *End) :
    NextBound(Start), End(End), ToLast(false), Done(false) {
    if (Start != End) {
      Current = *NextBound;
      ++NextBound;
      if (NextBound == End)
        ToLast = true;
    } else {
      Done = true;
    }
  }

  bool operator==(const ConstantRangeSetIterator &Other) const {
    // TODO: implement proper comparison operator
    revng_assert(Done or Other.Done);
    return Done == Other.Done;
  }

  bool operator!=(const ConstantRangeSetIterator &Other) const {
    return not(*this == Other);
  }

  ConstantRangeSetIterator &operator++() {
    revng_assert(not Done);

    if (ToLast and Current.isMaxValue()) {
      Done = true;
      return *this;
    }

    ++Current;
    if (not ToLast and Current == *NextBound) {
      ++NextBound;
      if (NextBound != End) {
        Current = *NextBound;
        ++NextBound;
        if (NextBound == End)
          ToLast = true;
      } else {
        Done = true;
      }
    }

    return *this;
  }

  const llvm::APInt &operator*() const {
    revng_assert(not Done);
    return Current;
  }

  const llvm::APInt *operator->() const { return &**this; }
};

/// \brief A set of ranges
///
/// This class is effectively an extension of llvm::ConstantRange aiming to
/// represent multiple disjoint ranges.
///
/// It is implemented as a vector of llvm::APInt. Each one of them represents a
/// flip in the status of the range (`ON -> OFF` or `OFF -> ON`), starting from
/// the initial state `OFF`.
class ConstantRangeSet {
private:
  APIntVector Bounds;
  uint32_t BitWidth;

public:
  ConstantRangeSet() : BitWidth(0) {}

  ConstantRangeSet(uint32_t BitWidth, bool IsFullSet) : BitWidth(BitWidth) {
    if (IsFullSet)
      Bounds.emplace_back(BitWidth, 0);
  }

  ConstantRangeSet(const llvm::ConstantRange &Range) {
    BitWidth = Range.getBitWidth();

    if (Range.isFullSet()) {
      Bounds.emplace_back(BitWidth, 0);
    } else if (Range.isEmptySet()) {
      // Nothing to do here
    } else if (Range.isWrappedSet()) {
      Bounds.emplace_back(BitWidth, 0);
      Bounds.push_back(Range.getUpper());
      Bounds.push_back(Range.getLower());
    } else {
      Bounds.push_back(Range.getLower());
      if (Range.getUpper() != llvm::APInt{ BitWidth, 0 })
        Bounds.push_back(Range.getUpper());
    }
  }

public:
  ConstantRangeSet unionWith(const ConstantRangeSet &Other) const {
    return merge<false>(Other);
  }

  ConstantRangeSet intersectWith(const ConstantRangeSet &Other) const {
    return merge<true>(Other);
  }

  bool contains(const ConstantRangeSet &Other) const {
    return intersectWith(Other) == Other;
  }

  bool operator==(const ConstantRangeSet &Other) const {
    return Bounds == Other.Bounds;
  }

  void setWidth(unsigned NewBitWidth) {
    if (BitWidth == 0)
      BitWidth = NewBitWidth;
    else
      revng_assert(BitWidth == NewBitWidth);
  }

  ConstantRangeSetIterator begin() const {
    return ConstantRangeSetIterator(&*Bounds.begin(), &*Bounds.end());
  }

  ConstantRangeSetIterator end() const {
    return ConstantRangeSetIterator(&*Bounds.end(), &*Bounds.end());
  }

  bool isFullSet() const {
    return Bounds.size() == 1 and Bounds[0].isNullValue();
  }
  bool isEmptySet() const { return Bounds.size() == 0; }

  llvm::APInt size() const {
    using namespace llvm;

    if (Bounds.size() == 0)
      return APInt(BitWidth, 0);

    APInt Size(BitWidth, 0);
    const APInt *Last = nullptr;
    for (const llvm::APInt &N : Bounds) {
      if (Last == nullptr) {
        Last = &N;
      } else {
        Size += (N - *Last);
        Last = nullptr;
      }
    }

    if (Last != nullptr)
      Size += (APInt::getMaxValue(BitWidth) - *Last);

    return Size;
  }

  void dump() const debug_function { dump(dbg); }

  template<typename T>
  void dump(T &Output) const {
    if (BitWidth == 0) {
      Output << "[)";
      return;
    }

    bool Open = true;
    for (const llvm::APInt &N : Bounds) {
      if (Open)
        Output << "[";
      else
        Output << ",";

      Output << N.getLimitedValue();

      if (not Open)
        Output << ") ";

      Open = not Open;
    }

    if (Bounds.size() == 0) {
      Output << "[)";
    }
    if (not Open) {
      Output << "," << llvm::APInt::getMaxValue(BitWidth).getLimitedValue()
             << ")";
    }
  }

private:
  template<bool And>
  ConstantRangeSet merge(const ConstantRangeSet &Other) const {
    using namespace llvm;

    auto ResultBitWidth = std::max(BitWidth, Other.BitWidth);
    ConstantRangeSet Result(ResultBitWidth, false);
    revng_assert(BitWidth == 0 or Other.BitWidth == 0
                 or BitWidth == Other.BitWidth);

    bool LastOutput = false;
    bool LeftActive = false;
    bool RightActive = false;
    auto zip_APIntVector = zipmap_range<const APIntVector,
                                        APIntVectorKeyContainer>;
    for (auto &P : zip_APIntVector(Bounds, Other.Bounds)) {
      const llvm::APInt *Left = P.first;
      const llvm::APInt *Right = P.second;

      if (Left != nullptr) {
        revng_assert(Left->getBitWidth() == ResultBitWidth);
        LeftActive = not LeftActive;
      }

      if (Right != nullptr) {
        revng_assert(Right->getBitWidth() == ResultBitWidth);
        RightActive = not RightActive;
      }

      const llvm::APInt *Value = P.first == nullptr ? P.second : P.first;
      revng_assert(Value != nullptr);

      bool NewOutput = And ? (LeftActive and RightActive) :
                             (LeftActive or RightActive);

      if (NewOutput != LastOutput) {
        Result.Bounds.push_back(*Value);
      }

      LastOutput = NewOutput;
    }

    return Result;
  }
};
