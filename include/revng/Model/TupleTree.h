#ifndef TUPLETREE_H
#define TUPLETREE_H

// Standard includes
#include <array>
#include <set>
#include <type_traits>
#include <vector>

// LLVM includes
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/YAMLTraits.h"

// Local libraries includes
#include "revng/ADT/KeyTraits.h"
#include "revng/ADT/KeyedObjectTraits.h"
#include "revng/Support/Assert.h"

//
// has_yaml
//
namespace tupletree::detail {

using namespace llvm::yaml;

template<typename T>
constexpr bool has_yaml_v =
  (has_DocumentListTraits<T>::value or has_MappingTraits<T, EmptyContext>::value
   or has_SequenceTraits<T>::value or has_BlockScalarTraits<T>::value
   or has_CustomMappingTraits<T>::value or has_PolymorphicTraits<T>::value
   or has_ScalarTraits<T>::value or has_ScalarEnumerationTraits<T>::value);

struct NoYaml {};

} // namespace tupletree::detail

template<typename T>
constexpr bool has_yaml_v = tupletree::detail::has_yaml_v<T>;

static_assert(!has_yaml_v<tupletree::detail::NoYaml>);
static_assert(has_yaml_v<int>);
static_assert(has_yaml_v<std::vector<int>>);

template<typename T, typename K = void>
using enable_if_has_yaml_t = std::enable_if_t<has_yaml_v<T>, K>;

template<typename T, typename K = void>
using enable_if_has_not_yaml_t = std::enable_if_t<not has_yaml_v<T>, K>;

//
// has_tuple_size
//
template<typename, typename = void>
struct has_tuple_size : std::false_type {};

template<typename T>
struct has_tuple_size<T, std::void_t<decltype(std::tuple_size<T>{})>>
  : std::true_type {};

template<typename T>
constexpr bool has_tuple_size_v = has_tuple_size<T>::value;

static_assert(has_tuple_size_v<std::tuple<>>);
static_assert(!has_tuple_size_v<std::vector<int>>);
static_assert(!has_tuple_size_v<int>);

template<size_t I, typename T, typename K = void>
using enable_if_tuple_end_t = std::enable_if_t<I == std::tuple_size_v<T>, K>;

template<size_t I, typename T, typename K = void>
using enable_if_not_tuple_end_t = std::enable_if_t<I != std::tuple_size_v<T>,
                                                   K>;

template<typename T, typename R = void>
using enable_if_has_tuple_size_t = std::enable_if_t<has_tuple_size_v<T>, R>;

//
// is_iterable
//
namespace tupletree::detail {

using std::begin;
using std::end;

// Require the following:
// * begin/end
// * operator!=
// * operator++
// * operator*
template<typename T>
decltype(begin(std::declval<T &>()) != end(std::declval<T &>()),
         ++std::declval<decltype(begin(std::declval<T &>())) &>(),
         void(*begin(std::declval<T &>())),
         std::true_type{})
is_iterable_impl(int);

template<typename T>
std::false_type is_iterable_impl(...);

} // namespace tupletree::detail

template<typename T>
using is_iterable = decltype(tupletree::detail::is_iterable_impl<T>(0));

template<typename T>
constexpr bool is_iterable_v = is_iterable<T>::value;

static_assert(is_iterable_v<std::vector<int>>);
static_assert(!is_iterable_v<int>);

//
// is_string_like
//
namespace tupletree::detail {
template<typename T>
typename std::is_same<decltype(std::declval<T>().c_str()), const char *>::type
has_c_str(int);

template<typename>
std::false_type has_c_str(...);

} // namespace tupletree::detail

template<typename T>
using has_c_str = typename decltype(tupletree::detail::has_c_str<T>(0))::type;

template<typename T>
constexpr bool has_c_str_v = has_c_str<T>::value;

static_assert(has_c_str_v<llvm::SmallString<4>>);
static_assert(!has_c_str_v<int>);

template<typename T>
constexpr bool
  is_string_like_v = (std::is_convertible_v<std::string, T> or has_c_str_v<T>);

static_assert(is_string_like_v<llvm::SmallString<4>>);
static_assert(is_string_like_v<std::string>);
static_assert(not is_string_like_v<llvm::ArrayRef<int>>);
static_assert(is_string_like_v<llvm::StringRef>);

//
// is_container
//
template<typename T>
constexpr bool is_container_v = is_iterable_v<T> and not is_string_like_v<T>;

template<typename T>
constexpr bool
  is_container_or_tuple_v = (is_container_v<T> or has_tuple_size_v<T>);

template<typename T, typename K = void>
using enable_if_is_not_container_or_tuple_t = std::
  enable_if_t<!is_container_or_tuple_v<T>, K>;

static_assert(is_container_v<std::vector<int>>);
static_assert(is_container_v<std::set<int>>);
static_assert(is_container_v<std::map<int, int>>);
static_assert(is_container_v<llvm::SmallVector<int, 4>>);
static_assert(!is_container_v<std::string>);
static_assert(!is_container_v<llvm::SmallString<4>>);
static_assert(!is_container_v<llvm::StringRef>);

template<typename T, typename K = void>
using enable_if_is_container_t = std::enable_if_t<is_container_v<T>, K>;

template<typename T, typename K = void>
using enable_if_is_not_container_t = std::enable_if_t<!is_container_v<T>, K>;

//
// slice
//

/// Copy into a std::array a slice of an llvm::ArrayRef
template<size_t Start, size_t Size, typename T>
std::array<T, Size> slice(llvm::ArrayRef<T> Old) {
  std::array<T, Size> Result;
  auto StartIt = Old.begin() + Start;
  std::copy(StartIt, StartIt + Size, Result.begin());
  return Result;
}

/// Copy into a std::array a slice of a std::array
template<size_t Start, size_t Size, typename T, size_t OldSize>
std::array<T, Size> slice(const std::array<T, OldSize> &Old) {
  std::array<T, Size> Result;
  auto StartIt = Old.begin() + Start;
  std::copy(StartIt, StartIt + Size, Result.begin());
  return Result;
}

//
// TupleLikeTraits
//
template<typename T>
struct TupleLikeTraits {
  // static const char *name();

  // template<size_t I=0>
  // static const char *fieldName();
};

//
// Implementation of MappingTraits for TupleLikeTraits implementors
//
template<typename T>
struct TupleLikeMappingTraits {
  // Recursive step
  template<size_t I = 0>
  static void mapping(llvm::yaml::IO &io, T &Obj) {
    // Define the field using getTupleFieldName and the associated field
    io.mapRequired(TupleLikeTraits<T>::template fieldName<I>(), get<I>(Obj));

    // Recur
    mapping<I + 1>(io, Obj);
  }

  // Base case
  template<>
  void mapping<std::tuple_size_v<T>>(llvm::yaml::IO &io, T &Obj) {}
};

//
// visit implementation
//

namespace tupletree::detail {

template<size_t I = 0, typename Visitor, typename T>
enable_if_tuple_end_t<I, T> visitTuple(Visitor &V, T &Obj) {
}

template<size_t I = 0, typename Visitor, typename T>
enable_if_not_tuple_end_t<I, T> visitTuple(Visitor &V, T &Obj) {
  // Visit the field
  visit(V, get<I>(Obj));

  // Visit next element in tuple
  visitTuple<I + 1>(V, Obj);
}

} // namespace tupletree::detail

// Tuple-like
template<typename Visitor, typename T>
enable_if_has_tuple_size_t<T, void> visit(Visitor &V, T &Obj) {
  V.preVisit(Obj);
  tupletree::detail::visitTuple(V, Obj);
  V.postVisit(Obj);
}

// Container-like
template<typename Visitor, typename T>
enable_if_is_container_t<T> visit(Visitor &V, T &Obj) {
  V.preVisit(Obj);
  using value_type = typename T::value_type;
  for (value_type &Element : Obj) {
    visit(V, Element);
  }
  V.postVisit(Obj);
}

// All the others
template<typename Visitor, typename T>
std::enable_if_t<not(is_container_v<T> or has_tuple_size_v<T>)>
visit(Visitor &V, T &Element) {
  V.preVisit(Element);
  V.postVisit(Element);
}

/// Default visitor, doing nothing
struct DefaultTupleTreeVisitor {
  template<typename T>
  void preVisit(T &) {}

  template<typename T>
  void postVisit(T &) {}
};

//
// tupleIndexByName
//
template<typename T, size_t I = 0>
enable_if_tuple_end_t<I, T, size_t> tupleIndexByName(llvm::StringRef Name) {
  return -1;
}

template<typename T, size_t I = 0>
enable_if_not_tuple_end_t<I, T, size_t> tupleIndexByName(llvm::StringRef Name) {
  llvm::StringRef ThisName = TupleLikeTraits<T>::template fieldName<I>();
  if (Name == ThisName)
    return I;
  else
    return tupleIndexByName<T, I + 1>(Name);
}

//
// getByKey
//
namespace tupletree::detail {

template<typename ResultT, size_t I = 0, typename RootT, typename KeyT>
enable_if_tuple_end_t<I, RootT, ResultT *> getByKeyTuple(RootT &M, KeyT Key) {
  return nullptr;
}

template<typename ResultT, size_t I = 0, typename RootT, typename KeyT>
enable_if_not_tuple_end_t<I, RootT, ResultT *>
getByKeyTuple(RootT &M, KeyT Key) {
  if (I == Key) {
    using tuple_element = typename std::tuple_element<I, RootT>::type;
    revng_assert((std::is_same_v<tuple_element, ResultT>) );
    return reinterpret_cast<ResultT *>(&get<I>(M));
  } else {
    return getByKeyTuple<ResultT, I + 1>(M, Key);
  }
}

} // namespace tupletree::detail

template<typename ResultT, typename RootT, typename KeyT>
enable_if_has_tuple_size_t<RootT, ResultT *> getByKey(RootT &M, KeyT Key) {
  return tupletree::detail::getByKeyTuple<ResultT>(M, Key);
}

template<typename ResultT, typename RootT, typename KeyT>
enable_if_is_container_t<RootT, ResultT *> getByKey(RootT &M, KeyT Key) {
  for (auto &Element : M) {
    using KOT = KeyedObjectTraits<std::remove_reference_t<decltype(Element)>>;
    if (KOT::key(Element) == Key)
      return &Element;
  }
  return nullptr;
}

//
// callOnPathSteps (no instance)
//
template<typename RootT, typename Visitor>
enable_if_has_tuple_size_t<RootT, void>
callOnPathSteps(Visitor &V, llvm::ArrayRef<KeyInt> Path);

template<typename RootT, typename Visitor>
enable_if_is_not_container_or_tuple_t<RootT>
callOnPathSteps(Visitor &V, llvm::ArrayRef<KeyInt> Path) {
  revng_abort();
}

template<typename RootT, typename Visitor>
enable_if_is_container_t<RootT>
callOnPathSteps(Visitor &V, llvm::ArrayRef<KeyInt> Path) {
  using value_type = typename RootT::value_type;
  using KOT = KeyedObjectTraits<value_type>;
  using key_type = decltype(KOT::key(std::declval<value_type>()));
  constexpr size_t IntsCount = KeyTraits<key_type>::IntsCount;
  auto PathStep = slice<0, IntsCount>(Path);
  auto TargetKey = KeyTraits<key_type>::fromInts(PathStep);

  V.template visitContainerElement<RootT>(TargetKey);
  if (Path.size() > IntsCount) {
    callOnPathSteps<typename RootT::value_type>(V, Path.slice(IntsCount));
  }
}

namespace tupletree::detail {

template<typename RootT, size_t I = 0, typename Visitor>
enable_if_tuple_end_t<I, RootT>
callOnPathStepsTuple(Visitor &V, llvm::ArrayRef<KeyInt> Path) {
}

template<typename RootT, size_t I = 0, typename Visitor>
enable_if_not_tuple_end_t<I, RootT>
callOnPathStepsTuple(Visitor &V, llvm::ArrayRef<KeyInt> Path) {
  if (Path[0] == I) {
    using next_type = typename std::tuple_element<I, RootT>::type;
    V.template visitTupleElement<RootT, I>();
    if (Path.size() > 1) {
      callOnPathSteps<next_type>(V, Path.slice(1));
    }
  } else {
    callOnPathStepsTuple<RootT, I + 1>(V, Path);
  }
}

} // namespace tupletree::detail

template<typename RootT, typename Visitor>
enable_if_has_tuple_size_t<RootT, void>
callOnPathSteps(Visitor &V, llvm::ArrayRef<KeyInt> Path) {
  tupletree::detail::callOnPathStepsTuple<RootT>(V, Path);
}

//
// callOnPathSteps (with instance)
//

namespace tupletree::detail {

template<size_t I = 0, typename RootT, typename Visitor>
enable_if_tuple_end_t<I, RootT>
callOnPathStepsTuple(Visitor &V, llvm::ArrayRef<KeyInt> Path, RootT &M) {
}

template<size_t I = 0, typename RootT, typename Visitor>
enable_if_not_tuple_end_t<I, RootT>
callOnPathStepsTuple(Visitor &V, llvm::ArrayRef<KeyInt> Path, RootT &M) {
  if (Path[0] == I) {
    using next_type = typename std::tuple_element<I, RootT>::type;
    next_type &Element = get<I>(M);
    V.template visitTupleElement<RootT, I>(Element);
    if (Path.size() > 1) {
      callOnPathSteps(V, Path.slice(1), Element);
    }
  } else {
    callOnPathStepsTuple<I + 1>(V, Path, M);
  }
}

} // namespace tupletree::detail

template<typename RootT, typename Visitor>
enable_if_has_tuple_size_t<RootT, void>
callOnPathSteps(Visitor &V, llvm::ArrayRef<KeyInt> Path, RootT &M) {
  tupletree::detail::callOnPathStepsTuple(V, Path, M);
}

template<typename RootT, typename Visitor>
enable_if_is_container_t<RootT>
callOnPathSteps(Visitor &V, llvm::ArrayRef<KeyInt> Path, RootT &M) {
  using value_type = typename RootT::value_type;
  using KOT = KeyedObjectTraits<value_type>;
  using key_type = decltype(KOT::key(std::declval<value_type>()));
  constexpr size_t IntsCount = KeyTraits<key_type>::IntsCount;
  auto PathStep = slice<0, IntsCount>(Path);
  auto TargetKey = KeyTraits<key_type>::fromInts(PathStep);

  value_type *Matching = nullptr;
  for (value_type &Element : M) {
    using KOT = KeyedObjectTraits<value_type>;
    if (KOT::key(Element) == TargetKey) {
      Matching = &Element;
      break;
    }
  }
  revng_check(Matching != nullptr);

  V.template visitContainerElement<RootT>(TargetKey, *Matching);
  if (Path.size() > IntsCount) {
    callOnPathSteps(V, Path.slice(IntsCount), *Matching);
  }
}

template<typename RootT, typename Visitor>
enable_if_is_not_container_or_tuple_t<RootT>
callOnPathSteps(Visitor &V, llvm::ArrayRef<KeyInt> Path, RootT &M) {
  revng_abort();
}

//
// callByPath (no instance)
//
namespace tupletree::detail {

template<typename Visitor>
struct CallByPathVisitor {
  size_t PathSize;
  Visitor &V;

  template<typename T, int I>
  void visitTupleElement() {
    --PathSize;
    if (PathSize == 0)
      V.template visitTupleElement<T, I>();
  }

  template<typename T, typename KeyT>
  void visitContainerElement(KeyT Key) {
    constexpr size_t IntsCount = KeyTraits<KeyT>::IntsCount;
    PathSize -= IntsCount;
    if (PathSize == 0)
      V.template visitContainerElement<T>(Key);
  }
};

} // namespace tupletree::detail

template<typename RootT, typename Visitor>
void callByPath(Visitor &V, const KeyIntVector &Path) {
  using namespace tupletree::detail;
  CallByPathVisitor<Visitor> CBPV{ Path.size(), V };
  callOnPathSteps<RootT>(CBPV, Path);
}

//
// callByPath (with instance)
//
namespace tupletree::detail {

template<typename Visitor>
struct CallByPathVisitorWithInstance {
  size_t PathSize;
  Visitor &V;

  template<typename T, size_t I, typename K>
  void visitTupleElement(K &Element) {
    --PathSize;
    if (PathSize == 0)
      V.template visitTupleElement<T, I>(Element);
  }

  template<typename T, typename K, typename KeyT>
  void visitContainerElement(KeyT Key, K &Element) {
    constexpr size_t IntsCount = KeyTraits<KeyT>::IntsCount;
    PathSize -= IntsCount;
    if (PathSize == 0)
      V.template visitContainerElement<T>(Key, Element);
  }
};

} // namespace tupletree::detail

template<typename RootT, typename Visitor>
void callByPath(Visitor &V, const KeyIntVector &Path, RootT &M) {
  using namespace tupletree::detail;
  CallByPathVisitorWithInstance<Visitor> CBPV{ Path.size(), V };
  callOnPathSteps(CBPV, Path, M);
}

//
// getByPath
//
namespace tupletree::detail {

template<typename ResultT>
struct GetByPathVisitor {
  ResultT *Result = nullptr;

  template<typename T, typename K, typename KeyT>
  void visitContainerElement(KeyT, K &) {
    Result = nullptr;
  }

  template<typename T, typename KeyT>
  void visitContainerElement(KeyT, ResultT &Element) {
    Result = &Element;
  }

  template<typename, size_t, typename K>
  void visitTupleElement(K &) {
    Result = nullptr;
  }

  template<typename, size_t>
  void visitTupleElement(ResultT &Element) {
    Result = &Element;
  }
};

} // namespace tupletree::detail

template<typename ResultT, typename RootT>
ResultT *getByPath(const KeyIntVector &Path, RootT &M) {
  using namespace tupletree::detail;
  GetByPathVisitor<ResultT> GBPV;
  callByPath(GBPV, Path, M);
  return GBPV.Result;
}

//
// pathAsString
//

namespace tupletree::detail {

class DumpPathVisitor {
private:
  llvm::raw_string_ostream Stream;

public:
  DumpPathVisitor(std::string &Result) : Stream(Result) {}

  template<typename T, int I>
  void visitTupleElement() {
    Stream << "/" << TupleLikeTraits<T>::template fieldName<I>();
  }

  template<typename T, typename KeyT>
  void visitContainerElement(KeyT Key) {
    Stream << "/" << KeyTraits<KeyT>::toString(Key);
  }
};

} // namespace tupletree::detail

template<typename T>
std::string pathAsString(const KeyIntVector &Path) {
  std::string Result;
  {
    tupletree::detail::DumpPathVisitor PV(Result);
    callOnPathSteps<T>(PV, Path);
  }
  return Result;
}

//
// FOR_EACH macro implemenation
//
#define GET_MACRO(_0, _1, _2, _3, _4, _5, NAME, ...) NAME
#define NUMARGS(...) GET_MACRO(_0, __VA_ARGS__, 5, 4, 3, 2, 1)

#define FE_0(ACTION, TOTAL, ARG)

#define FE_1(ACTION, TOTAL, ARG, X) ACTION(ARG, (TOTAL) -0, X)

#define FE_2(ACTION, TOTAL, ARG, X, ...) \
  ACTION(ARG, (TOTAL) -1, X)             \
  FE_1(ACTION, TOTAL, ARG, __VA_ARGS__)

#define FE_3(ACTION, TOTAL, ARG, X, ...) \
  ACTION(ARG, (TOTAL) -2, X)             \
  FE_2(ACTION, TOTAL, ARG, __VA_ARGS__)

#define FE_4(ACTION, TOTAL, ARG, X, ...) \
  ACTION(ARG, (TOTAL) -3, X)             \
  FE_3(ACTION, TOTAL, ARG, __VA_ARGS__)

#define FE_5(ACTION, TOTAL, ARG, X, ...) \
  ACTION(ARG, (TOTAL) -4, X)             \
  FE_4(ACTION, TOTAL, ARG, __VA_ARGS__)

/// Calls ACTION(ARG, INDEX, VA_ARG) for each VA_ARG in ...
#define FOR_EACH(ACTION, ARG, ...)                               \
  GET_MACRO(_0, __VA_ARGS__, FE_5, FE_4, FE_3, FE_2, FE_1, FE_0) \
  (ACTION, (NUMARGS(__VA_ARGS__) - 1), ARG, __VA_ARGS__)

//
// Macros to transform struct in tuple-like
//
#define TUPLE_ELEMENTS(class, index, field) \
  template<>                                \
  struct std::tuple_element<index, class> { \
    using type = decltype(class ::field);   \
  };

#define GET_IMPLEMENTATIONS(class, index, field) \
  else if constexpr (I == index) return x.field;

#define GET_TUPLE_FIELD_NAME(class, index, field) \
  template<>                                      \
  const char *fieldName<index>() {                \
    return #field;                                \
  }

#define INTROSPECTION_1(class, ...)                            \
  template<>                                                   \
  struct std::tuple_size<class>                                \
    : std::integral_constant<size_t, NUMARGS(__VA_ARGS__)> {}; \
                                                               \
  FOR_EACH(TUPLE_ELEMENTS, class, __VA_ARGS__)                 \
                                                               \
  template<>                                                   \
  struct TupleLikeTraits<class> {                              \
    static const char *name() { return #class; }               \
                                                               \
    template<size_t I = 0>                                     \
    static const char *fieldName();                            \
                                                               \
    FOR_EACH(GET_TUPLE_FIELD_NAME, class, __VA_ARGS__)         \
  };

#define INTROSPECTION_2(class, ...)                   \
  template<int I>                                     \
  auto &get(class &&x) {                              \
    if constexpr (false)                              \
      return NULL;                                    \
    FOR_EACH(GET_IMPLEMENTATIONS, class, __VA_ARGS__) \
  }                                                   \
                                                      \
  template<int I>                                     \
  const auto &get(const class &x) {                   \
    if constexpr (false)                              \
      return NULL;                                    \
    FOR_EACH(GET_IMPLEMENTATIONS, class, __VA_ARGS__) \
  }                                                   \
                                                      \
  template<int I>                                     \
  auto &get(class &x) {                               \
    if constexpr (false)                              \
      return NULL;                                    \
    FOR_EACH(GET_IMPLEMENTATIONS, class, __VA_ARGS__) \
  }

#define INTROSPECTION(class, ...)     \
  INTROSPECTION_1(class, __VA_ARGS__) \
  INTROSPECTION_2(class, __VA_ARGS__)

#define INTROSPECTION_NS(ns, class, ...)  \
  INTROSPECTION_1(ns::class, __VA_ARGS__) \
  namespace ns {                          \
  INTROSPECTION_2(class, __VA_ARGS__)     \
  }

#endif // TUPLETREE_H
