#ifndef TUPLESPACE_H
#define TUPLESPACE_H

#include <concepts>
#include <tuple>
#include <ostream>

template<class... DIMTYPES>
class TupleSpace : public std::tuple<DIMTYPES...> {
public:
    typedef std::tuple_element_t<0,std::tuple<DIMTYPES...>> Time;

    static constexpr size_t DIMENSIONS = sizeof...(DIMTYPES);

    TupleSpace(DIMTYPES... values) : std::tuple<DIMTYPES...>(values...) { }

    TupleSpace() {
        visit([](auto &x) { x = 0; });
    }

    TupleSpace(Time t) : TupleSpace() {
        std::get<0>(*this) = t;
    }

    TupleSpace<DIMTYPES...> operator +(const TupleSpace<DIMTYPES...> &other) const {
        return callWithIndices<DIMENSIONS>([this, &other]<size_t...I>() {
            return TupleSpace((std::get<I>(*this) + std::get<I>(other))...);
        });
    }

    TupleSpace<DIMTYPES...> operator -(const TupleSpace<DIMTYPES...> &other) const {
        return callWithIndices<DIMENSIONS>([this, &other]<size_t...I>() {
            return TupleSpace((std::get<I>(*this) - std::get<I>(other))...);
        });
    }

    TupleSpace<DIMTYPES...> operator *(Time t) const {
        return callWithIndices<DIMENSIONS>([this, t]<size_t...I>() {
            return TupleSpace((std::get<I>(*this) * t)...);
        });
    }


    Time operator *(const TupleSpace<DIMTYPES...> &other) const {
        return callWithIndices<DIMENSIONS-1>([this,&other]<size_t...I>() {
            return ((std::get<0>(*this) * std::get<0>(other)) - ... - (std::get<I+1>(*this) * std::get<I+1>(other)));
        });
    }


    friend std::ostream &operator <<(std::ostream &out, const TupleSpace<DIMTYPES...> &tSpace) {
        out << "( ";
        callWithIndices<sizeof...(DIMTYPES)>([&out, &tSpace]<size_t...I>() { 
            ((out << std::get<I>(tSpace) << " "),...);
        });
        out << ")";
        return out;
    }

protected:

    template<class LAMBDA>
    inline auto visit(const TupleSpace<DIMTYPES...> &other, LAMBDA &&lambda) const {
        return [this, &other, f = std::forward<LAMBDA>(lambda)]<size_t...I>(std::index_sequence<I...>) {
            return TupleSpace(f(std::get<I>(*this), std::get<I>(other))...);
        }(std::index_sequence_for<DIMTYPES...>());
    }

    template<class LAMBDA>
    inline auto visit(LAMBDA &&lambda) const {
        return [this, f = std::forward<LAMBDA>(lambda)]<size_t...I>(std::index_sequence<I...>) {
            return TupleSpace(f(std::get<I>(*this))...);
        }(std::index_sequence_for<DIMTYPES...>());
    }

    template<class LAMBDA> requires std::same_as<void, decltype(std::declval<LAMBDA>()(std::declval<const Time &>()))>
    inline void visit(LAMBDA &&lambda) const {
        [this, f = std::forward<LAMBDA>(lambda)]<size_t...I>(std::index_sequence<I...>) {
            (f(std::get<I>(*this)),...);
        }(std::index_sequence_for<DIMTYPES...>());
    }

    template<class LAMBDA> requires std::same_as<void, decltype(std::declval<LAMBDA>()(std::declval<Time &>()))>
    inline void visit(LAMBDA &&lambda) {
        [this, f = std::forward<LAMBDA>(lambda)]<size_t...I>(std::index_sequence<I...>) {
            (f(std::get<I>(*this)),...);
        }(std::index_sequence_for<DIMTYPES...>());
    }

    template<class LAMBDA>
    inline void expandRight(LAMBDA &&lambda) {
        [this, f = std::forward<LAMBDA>(lambda)]<size_t...I>(std::index_sequence<I...>) {
            (f.template operator()<I>(*this),...);
        }(std::index_sequence_for<DIMTYPES...>());
    }



};


// template<size_t SIZE, class LAMBDA>
// void forEachIn(LAMBDA &&templatedLambda) {
//     []<size_t...INDICES>()
//     forEachIn(std::make_index_sequence<SIZE>(), std::forward<LAMBDA>(lambda));
// }


// template<class LAMBDA, size_t...INDICES>
// void forEachIn(std::index_sequence<INDICES...>, LAMBDA &&templatedLambda) {
//     (templatedLambda.template operator()<INDICES>(),...);
// }

// // template<class LAMBDA, class... TS>
// // void fold(std::tuple<TS...> &t, LAMBDA &&templatedLambda) {
// //     fold(std::index_sequence_for<TS>(), t, std::forward<LAMBDA>(lambda));
// // }


template<size_t SIZE, class LAMBDA, class... ARGS>
inline auto callWithIndices(LAMBDA &&templatedLambda, ARGS &&... args) {
    return callWithTemplates(std::make_index_sequence<SIZE>(), std::forward<LAMBDA>(templatedLambda), std::forward<ARGS>(args)...);
}

template<class... TYPES, class LAMBDA, class... ARGS>
inline auto callWithIndicesFor(LAMBDA &&templatedLambda, ARGS &&... args) {
    return callWithTemplates(std::index_sequence_for<TYPES...>(), std::forward<LAMBDA>(templatedLambda), std::forward<ARGS>(args)...);
}


template<class LAMBDA, size_t...INDICES, class... ARGS>
inline auto callWithTemplates(std::index_sequence<INDICES...>, LAMBDA &&templatedLambda, ARGS &&...args) {
    return templatedLambda.template operator()<INDICES...>(std::forward<ARGS>(args)...);
}


template<class...TYPES, class FUNCTION>
void for_each(std::tuple<TYPES...> &tuple, FUNCTION &&func) {
//    (func(std::get<INDICES>(tuple)), ...);
}

template<size_t... INDICES, class...TYPES, class FUNCTION>
inline void for_each(std::index_sequence<INDICES...>, std::tuple<TYPES...> &tuple, FUNCTION &&func) {
    (func(std::get<INDICES>(tuple)), ...);
}

template<class INDICES, class TUPLE>
class IndexedTuple {
public:
//    IndexedTuple(TUPLE t) {}
};

template<size_t... INDICES, class... TS>
class IndexedTuple<std::index_sequence<INDICES...>, std::tuple<TS...>> : public std::tuple<TS...> {
public:
    IndexedTuple(TS &&... ts) : std::tuple<TS...>(std::forward<TS>(ts)...) {}

    static void myFunc() {}
};

template<class...TYPES>
IndexedTuple(TYPES... ts) -> IndexedTuple<decltype(std::index_sequence_for<TYPES...>()),std::tuple<TYPES...>>;


template<class... TS>
using IndexedTupleFor = decltype(IndexedTuple(std::declval<TS>()...));


#endif
