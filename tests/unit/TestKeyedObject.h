#ifndef TESTKEYEDOBJECT_H
#define TESTKEYEDOBJECT_H

//
// This file is distributed under the MIT License. See LICENSE.md for details.
//

// Local libraries includes
#include "revng/ADT/KeyedObjectTraits.h"

struct Element {
  uint64_t Key;
  uint64_t Value;

  Element(uint64_t Key) : Key(Key), Value(0) {}
  Element(uint64_t Key, uint64_t Value) : Key(Key), Value(Value) {}

  uint64_t key() const { return Key; }
  uint64_t value() const { return Value; }
  void setValue(uint64_t NewValue) { Value = NewValue; }
};

template<>
struct KeyedObjectTraits<Element> {
  static uint64_t key(const Element &SE) { return SE.key(); }
  static Element fromKey(uint64_t Key) { return Element(Key); }
};

#endif // TESTKEYEDOBJECT_H
